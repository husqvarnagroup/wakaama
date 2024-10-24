#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

use std::cmp::min;
use std::sync::{Arc, Mutex};

include!(concat!(env!("OUT_DIR"), "/wakaama_bindings.rs"));

#[no_mangle]
pub extern "C" fn lwm2m_session_is_equal(
    session1: *mut ::std::os::raw::c_void,
    session2: *mut ::std::os::raw::c_void,
    _userData: *mut ::std::os::raw::c_void,
) -> bool {
    let session1 = session1 as *const i32;
    let session2 = session2 as *const i32;
    session1 == session2
}

//
const MAX_PACKET_SIZE: usize = 2048;

lazy_static::lazy_static! {
    static ref SEND_BUF_MUTEX: Arc<Mutex<Vec<u8>>> = Arc::new(Mutex::new(vec![]));
}

/// # Safety
///
/// This function is called by Wakaama internally. Don't call it manually.
#[no_mangle]
pub unsafe extern "C" fn lwm2m_buffer_send(
    _sessionH: *mut ::std::os::raw::c_void,
    buffer: *mut u8,
    length: usize,
    _userData: *mut ::std::os::raw::c_void,
) -> u8 {
    let length = min(length, MAX_PACKET_SIZE);
    let mut buf = SEND_BUF_MUTEX.lock().unwrap();
    buf.clear();
    buf.resize(length, 0);
    unsafe {
        buf.copy_from_slice(std::slice::from_raw_parts(buffer, length));
    }
    COAP_NO_ERROR as u8
}


pub struct Server {
    context: *mut lwm2m_context_t
}

impl Server {
    pub fn new() -> Server {
        Server { context: unsafe { lwm2m_init(std::ptr::null_mut()) } }
    }

    pub fn set_monitoring_callback(&self, monitoring_callback: lwm2m_result_callback_t) {
        unsafe {
            let user_data = std::ptr::null_mut();
            lwm2m_set_monitoring_callback(self.context, monitoring_callback, user_data);
        }
    }

    fn handle_packet(self, mut buffer: Vec<u8>) {
        unsafe {
            let buf = buffer.as_mut_ptr();
            let len = buffer.len();
            let session = std::ptr::null_mut();
            lwm2m_handle_packet(self.context, buf, len, session);
        }
    }
}

extern "C" fn monitoring_callback(
    _contextP: *mut lwm2m_context_t,
    clientID: u16,
    _uriP: *mut lwm2m_uri_t,
    _status: ::std::os::raw::c_int,
    _block_info: *mut block_info_t,
    _format: lwm2m_media_type_t,
    _data: *mut u8,
    _dataLength: usize,
    _userData: *mut ::std::os::raw::c_void,
) {
    println!("monitoring_callback called with id: {}", clientID);
}

#[cfg(test)]
mod tests {
    use super::*;

    use coap_lite::ResponseType::Created;
    use coap_lite::{
        CoapOption, CoapRequest, ContentFormat, MessageClass, Packet, RequestType as Method,
    };
    use std::net::SocketAddr;

    #[test]
    fn test_handle_packet() {
        
        let server = Server::new();
        server.set_monitoring_callback(Some(monitoring_callback));
        

        let packet = coap_client_for_tests();
        server.handle_packet(packet);

        let response = get_response_from_wakaama();

        assert_eq!(response.header.code, MessageClass::Response(Created));
    }

    fn get_response_from_wakaama() -> Packet {
        let actual = SEND_BUF_MUTEX.lock().unwrap();
        let actual = actual.as_slice();
        Packet::from_bytes(actual).unwrap()
    }

    fn coap_client_for_tests() -> Vec<u8> {
        let mut request: CoapRequest<SocketAddr> = CoapRequest::new();

        request.set_method(Method::Post);
        request.set_path("/rd");
        request.message.add_option(
            CoapOption::UriQuery,
            "ep=rs-test-client".as_bytes().to_vec(),
        );
        request
            .message
            .add_option(CoapOption::UriQuery, "lt=43200".as_bytes().to_vec());
        request
            .message
            .add_option(CoapOption::UriQuery, "lwm2m=1.1".as_bytes().to_vec());
        request
            .message
            .add_option(CoapOption::UriQuery, "b=U".as_bytes().to_vec());
        request.message.payload = b"</1/1>,</2/1>,</3/0>".to_vec();
        request
            .message
            .set_content_format(ContentFormat::ApplicationLinkFormat);

        request.message.to_bytes().unwrap()
    }
}
