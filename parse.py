import subprocess
import sys
import argparse

def write_definitions(def_file, definitions, arch):
    write_info = '''#pragma once

/*
{} - {}
*/
{}'''.format(args.dll, arch, definitions)
    try:
        with open(def_file, "w") as write_file:
            write_file.write(write_info)
        write_file.close()
    except Exception as e:
        print("[-] Write File Error: '{}'".format(e))
        return False
    return True

def run_cmd(cmd):
    result = subprocess.run(cmd, stdout=subprocess.PIPE, shell=True)
    return result.stdout.decode('utf-8').strip()

def remove_empty(array):
    return ' '.join(array.split()).split()

def create_defs():
    arch = " ".join(remove_empty(run_cmd('"{}" /HEADERS "{}" | findstr "machine"'.format(args.dump_bin, args.dll)).replace("\r\n", " | ")))
    if arch:
        print("[+] Detected DLL Architecture: '{}'".format(arch))

    definitions=""
    num_forwarded=0
    num_defs=0
    is_exports=False
    export = run_cmd('"{}" /EXPORTS "{}"'.format(args.dump_bin, args.dll))
    for raw_line in export.splitlines():
        line = remove_empty(raw_line)
        if line:
            if line == ['ordinal', 'hint', 'RVA', 'name']:
                is_exports=True
            elif line == ['Summary']:
                is_exports=False
            elif is_exports:
                dll_redirect_name = real_dll_name
                # detected proxied dll functions, lets keep it as it
                if raw_line.find("(forwarded") != -1:
                    exported_name = raw_line.split("(forwarded to ")[1].replace(")", "")
                    function_name = exported_name
                    dll_redirect_name = function_name.split(".")[0]
                    num_forwarded+=1
                ordinal = line[0]
                if len(line) == 3:
                    # function name doesnt matter as its only by ordinal ( no function name is documented)
                    function_name = "dontmatter"
                elif len(line) == 4:
                    function_name = line[-1]
                if function_name:
                    definitions += ("\n#pragma comment(linker,\"/export:%s=%s.%s,@%s\")" % (function_name, dll_redirect_name, function_name, ordinal))
                num_defs +=1

    if num_defs:
        if num_forwarded == num_defs:
            print("[-] DLL Only Contained Proxy Calls to other DLLS unable to give reliable definitions")
        else:
            print("[+] Made '{}' Definitions".format(num_defs))
            if write_definitions(args.header_file, definitions, arch):
                print("[+] Successfully Able to Write to File: '{}'".format(args.header_file))
    else:
        print("[-] No Definitions")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Proxy DLL Creator')

    parser.add_argument(
        '-d',
        action="store", dest="dll",
        help='Path to DLL',
        required=True
    )
    parser.add_argument(
        '-f',
        action="store", dest="header_file",
        default="./malicious_dll/definitions.h",
        help='Path to created definitions Header File'
    )

    parser.add_argument(
        '-b',
        action="store", dest="dump_bin", 
        default="C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\VC\\Tools\\MSVC\\14.24.28314\\bin\\Hostx64\\x64\\dumpbin.exe",
        help='Path to Dumpbin Binary'
    )

    args = parser.parse_args()
    real_dll_name = args.dll.replace(".dll", "_").split("\\")[-1].split("/")[-1]
    create_defs()