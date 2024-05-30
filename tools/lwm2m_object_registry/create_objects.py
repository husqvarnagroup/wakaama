#!/usr/bin/env python3
import glob
import os
import sys
from dataclasses import dataclass
from pathlib import Path
from textwrap import indent
import xml.etree.ElementTree as ET
from typing import List


script_dir = Path(__file__).parent
repo_base_dir = script_dir / "../.."
script_name = Path(__file__).name


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


def generate_object_code(object):

    code = ""

    code += "lwm2m_object_definition_t *obj = lwm2m_malloc(sizeof(lwm2m_object_definition_t));\n";
    code += "memset(obj, 0, sizeof(lwm2m_object_definition_t));\n\n\n";

    code += "lwm2m_registry_init_object(obj,\n"
    init_func_args = [
        object.object_id ,
        f'"{object.object_name}"',
        f'"{object.object_urn}"',
        convert_lwm2m_version_to_enum(object.lwm2m_version),
        object.object_version_major,
        object.object_version_minor,
        convert_multiple_instances_to_bool(object.multiple_instances),
        convert_mandatory_to_bool(object.mandatory)]

    init_func_args_code = ",\n".join(init_func_args)
    init_func_args_code = indent(init_func_args_code, "\t")
    code += init_func_args_code
    code += ");\n"

    return code


def generate_resource_code(resources):
    code = ""
    for res in resources:
        code += f'\n\n/* Resource: "{res.resource_name}" */\n'
        code += "lwm2m_registry_add_object_resource(obj,\n"

        resource_funct_args = [
            res.resource_id,
            f'"{res.resource_name}"',
            convert_resource_operations_to_enum(res.operations),
            convert_multiple_instances_to_bool(res.multiple_instances),
            convert_mandatory_to_bool(res.mandatory),
            convert_resource_type_to_enum(res.resource_type)
        ]
        args_code = ",\n".join(resource_funct_args)
        args_code = indent(args_code, "\t")

        code += args_code
        code += ");\n"

    return code


def object_creation_function_comment(object):
    return (f'/* "{object.object_name}" '
            f"({object.object_id}:{object.object_version_major}.{object.object_version_minor}) */")


def object_creation_function_name(object):
    return (f"create_object_{object.object_id}_version_"
            f"{object.object_version_major}_{object.object_version_minor}")


def generate_object_creation_function(object):

    code = object_creation_function_comment(object) + "\n"

    code += (f"static lwm2m_object_definition_t* "
             f"{object_creation_function_name(object)}(void)\n")

    code += "{\n"
    code += indent(generate_object_code(object), "\t");
    code += indent(generate_resource_code(object.resources), "\t")

    code += indent("\nreturn obj;", '\t')

    code += "\n}\n\n"

    return code


def generate_public_registry_initialization_function(object):
    code = "lwm2m_object_definition_list_t* lwm2m_registry_initialize(void)\n"
    code += "{\n"

    body = "lwm2m_object_definition_list_t * head = lwm2m_malloc(sizeof(lwm2m_object_definition_list_t));\n"
    body += "memset(head, 0, sizeof(lwm2m_object_definition_list_t));\n\n"

    body += object_creation_function_comment(object) + "\n"
    body += f"head->object = {object_creation_function_name(object)}();\n\n"

    body += "return head;\n"

    code += indent(body, "\t")
    code += "}\n"

    return code



# DTOs


@dataclass
class Lwm2mObjectResource:
    resource_id: int
    resource_name: str
    operations: str
    multiple_instances: str
    mandatory: str
    resource_type: str

    def __init__(self):
        pass


@dataclass
class Lwm2mObject:
    object_id: int
    object_name: str
    object_urn: str
    lwm2m_version: str
    object_version_major: int
    object_version_minor: int
    multiple_instances: str
    mandatory: str
    resources: List[Lwm2mObjectResource]

    def __init__(self):
        pass

# Parsing

def parse_all_files(dir):
    files = glob.glob(f"{dir}/*.xml")
    objects = []
    for file in files:
        objects.append(parse_file(file))
    objects.sort(key=lambda obj: obj.object_id)
    return objects


def parse_file(definition_xml):
    tree = ET.parse(definition_xml)
    root = tree.getroot()
    assert root.tag == "LWM2M"
    object_tag = root.find("Object")

    return parse_object(object_tag)


def parse_object(object_tag):
    assert object_tag.tag == "Object"
    obj = Lwm2mObject()
    obj.object_id = object_tag.find("ObjectID").text
    obj.object_name = object_tag.find("Name").text
    obj.object_urn = object_tag.find("ObjectURN").text
    obj.lwm2m_version = object_tag.find("LWM2MVersion").text
    obj.object_version_major, obj.object_version_minor = object_version_tuple(object_tag)

    obj.multiple_instances = object_tag.find("MultipleInstances").text
    obj.mandatory = object_tag.find("Mandatory").text

    obj.resources = parse_resources(object_tag.find("Resources"))
    obj.resources.sort(key=lambda res: res.resource_id)

    return obj


### Parse Resource

def parse_resources(resources_tag):
    resource_list = []
    for item in resources_tag:
        assert item.tag == "Item"

        r = Lwm2mObjectResource()
        r.resource_id = item.get("ID")
        r.resource_name = item.find("Name").text
        r.operations = item.find("Operations").text
        r.multiple_instances = item.find("MultipleInstances").text
        r.mandatory = item.find("Mandatory").text
        r.resource_type = item.find("Type").text
        resource_list.append(r)

    return resource_list




def parse_definitions_and_generate_code(definitions_directory):
    objects = parse_all_files(definitions_directory)

    code = f"/* Generated by {script_name} */\n"

    code += '#include "liblwm2m.h"\n#include <memory.h>\n\n'

    for obj in objects:

        code += generate_object_creation_function(obj)

        code += generate_public_registry_initialization_function(obj)

    return code


def main():
    definitions_directory = repo_base_dir / "resources" / "object_definitions"
    code = parse_definitions_and_generate_code(definitions_directory)
    output_dir = repo_base_dir / "object_registry/objects"
    os.makedirs(output_dir, exist_ok=True)
    with open(output_dir / "lwm2m_objects.c", 'w') as o_file:
        o_file.write(code)

    #TODO Remove
    return code



import unittest

from approvaltests.approvals import verify
from approvaltests.reporters.default_reporter_factory import set_default_reporter
from approvaltests.reporters.report_with_vscode import ReportWithVSCode
from approvaltests.core.options import Options


set_default_reporter(ReportWithVSCode())


class GettingStartedTest(unittest.TestCase):
    def test_simple(self):
        main()
        options = Options().for_file.with_extension(".c")
        verify(main(), options=options)


if __name__ == "__main__":
    unittest.main()
