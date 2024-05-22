#!/usr/bin/env python3

from pathlib import Path
from textwrap import indent
import xml.etree.ElementTree as ET


def convert_lwm2m_version_to_enum(lwm2m_version):
    if lwm2m_version == "1.0":
        return "VERSION_1_0"
    elif lwm2m_version == "1.1":
        return "VERSION_1_1"
    return "VERSION_UNRECOGNIZED"


def convert_resource_operations_to_enum(operations):
    if operations == "R":
        return "LWM2M_RESOURCES_OPERATIONS_READ"
    elif operations == "W":
        return "LWM2M_RESOURCES_OPERATIONS_WRITE"
    elif operations == "RW":
        return "LWM2M_RESOURCES_OPERATIONS_READ_WRITE"
    elif operations == "E":
        return "LWM2M_RESOURCES_OPERATIONS_EXECUTE"
    return "LWM2M_RESOURCES_OPERATIONS_NONE"


def convert_resource_type_to_enum(lwm2m_type):
    if lwm2m_type == "String":
        return "LWM2M_TYPE_STRING"
    elif lwm2m_type == "Integer":
        return "LWM2M_TYPE_INTEGER"
    elif lwm2m_type == "Float":
        return "LWM2M_TYPE_FLOAT"
    elif lwm2m_type == "Boolean":
        return "LWM2M_TYPE_BOOLEAN"
    elif lwm2m_type == "Opaque":
        return "LWM2M_TYPE_OPAQUE"
    elif lwm2m_type == "Time":
        return "LWM2M_TYPE_TIME"
    elif lwm2m_type == "Objlnk":
        return "LWM2M_TYPE_OBJECT_LINK"
    elif lwm2m_type == "Unsigned Integer":
        return "LWM2M_TYPE_UNSIGNED_INTEGER"
    elif lwm2m_type == "Corelnk":
        return "LWM2M_TYPE_CORE_LINK"
    return "LWM2M_TYPE_UNDEFINED"


def convert_multiple_instances_to_bool(multiple_instances):
    return 'true' if multiple_instances == 'Multiple' else 'false'


def convert_mandatory_to_bool(mandatory):
    return 'true' if mandatory == 'Mandatory' else 'false'


def object_version_tuple(lwm2m_object):
    assert lwm2m_object.tag == "Object"
    object_version = lwm2m_object.find("ObjectVersion").text
    object_version = object_version.split(".")
    assert len(object_version) == 2
    object_version_major, object_version_minor = object_version
    return object_version_major, object_version_minor


def generate_object_code(lwm2m_object):
    assert lwm2m_object.tag == "Object"
    object_id = lwm2m_object.find("ObjectID").text
    object_name = f'"{lwm2m_object.find("Name").text}"'
    object_urn = f'"{lwm2m_object.find("ObjectURN").text}"'
    lwm2m_version = lwm2m_object.find("LWM2MVersion").text
    object_version_major, object_version_minor = object_version_tuple(lwm2m_object)

    multiple_instances = lwm2m_object.find("MultipleInstances").text
    mandatory = lwm2m_object.find("Mandatory").text

    code = ""

    code += "static lwm2m_object_definition_t obj;\n\n\n";

    code += "lwm2m_registry_init_object(&obj,\n"
    init_func_args = [
        object_id ,
        object_name,
        object_urn,
        convert_lwm2m_version_to_enum(lwm2m_version),
        object_version_major,
        object_version_minor,
        convert_multiple_instances_to_bool(multiple_instances),
        convert_mandatory_to_bool(mandatory)]

    init_func_args_code = ",\n".join(init_func_args)
    init_func_args_code = indent(init_func_args_code, "\t")
    code += init_func_args_code
    code += ");\n"

    return code


def generate_resource_code(resources):
    assert resources.tag == "Resources"
    code = ""
    for res in resources:
        assert res.tag == "Item"

        resource_id = res.get("ID")
        resource_name = res.find("Name").text
        resource_name = f'"{resource_name}"'
        operations = res.find("Operations").text
        multiple_instances = res.find("MultipleInstances").text
        mandatory = res.find("Mandatory").text
        resource_type = res.find("Type").text

        code += f"\n\n/* Resource: {resource_name} */\n"
        code += "lwm2m_registry_add_object_resource(&obj,\n"

        resouce_funct_args = [
            resource_id,
            resource_name,
            convert_resource_operations_to_enum(operations),
            convert_multiple_instances_to_bool(multiple_instances),
            convert_mandatory_to_bool(mandatory),
            convert_resource_type_to_enum(resource_type)
        ]
        args_code = ",\n".join(resouce_funct_args)
        args_code = indent(args_code, "\t")

        code += args_code
        code += ");\n"

    return code


def object_creation_function_comment(lwm2m_object):
    object_name = f'"{lwm2m_object.find("Name").text}"'
    object_id = lwm2m_object.find("ObjectID").text
    object_version_major, object_version_minor = object_version_tuple(lwm2m_object)
    return f"/* {object_name} ({object_id}:{object_version_major}.{object_version_minor}) */"


def object_creation_function_name(lwm2m_object):
    object_id = lwm2m_object.find("ObjectID").text
    object_version_major, object_version_minor = object_version_tuple(lwm2m_object)
    return f"create_object_{object_id}_version_{object_version_major}_{object_version_minor}"


def generate_object_creation_function(lwm2m_object):

    code = object_creation_function_comment(lwm2m_object) + "\n"

    code += (f"static lwm2m_object_definition_t* "
             f"{object_creation_function_name(lwm2m_object)}(void)\n")

    code += "{\n"
    code += indent(generate_object_code(lwm2m_object), "\t");
    code += indent(generate_resource_code(lwm2m_object.find("Resources")), "\t")

    code += "}\n\n"

    return code


def generate_public_registry_initialization_function(lwm2m_object):
    code = "lwm2m_object_definition_t* lwm2m_registry_initalize(void)\n"
    code += "{\n"

    body = "lwm2m_object_definition_t * head = NULL;\n\n"
    body += "lwm2m_object_definition_t * obj = NULL;\n\n"

    body += object_creation_function_comment(lwm2m_object) + "\n"
    body += f"obj = {object_creation_function_name(lwm2m_object)}();\n"
    body += f"obj->id = lwm2m_list_newId(obj);\n"
    body += f"obj->id = LWM2M_LIST_ADD(head, obj);\n\n"

    body += "return head;\n"

    code += indent(body, "\t")
    code += "}\n"

    return code



definition_xml = "/home/lukas/projects/sg-bnw-ipso-registry/definitions/0-1_1.xml"

tree = ET.parse(definition_xml)
root = tree.getroot()
assert root.tag == "LWM2M"

lwm2m_object = root.find("Object")

script_name = Path(__file__).name
code = f"/* Generated by {script_name} */\n"

code += '#include "liblwm2m.h"\n\n'

code += generate_object_creation_function(lwm2m_object)

code += generate_public_registry_initialization_function(lwm2m_object)




import unittest

from approvaltests.approvals import verify
from approvaltests.reporters.default_reporter_factory import set_default_reporter
from approvaltests.reporters.report_with_vscode import ReportWithVSCode
from approvaltests.core.options import Options


set_default_reporter(ReportWithVSCode())


class GettingStartedTest(unittest.TestCase):
    def test_simple(self):
        options = Options().for_file.with_extension(".c")
        verify(code, options=options)


if __name__ == "__main__":
    unittest.main()
