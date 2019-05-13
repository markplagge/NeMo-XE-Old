import os
import sys
import re

def create_constexpr(line):
    name_r = r'[A-Z_]+'
    rr =  re.search(name_r,line)
    if rr != None:
        name = rr[0]

        csp = "constexpr static int " + name.replace("DEF_","") + " = " + name + ";\n"
    else:
        csp = ""
    return csp

def create_constbool(line):
    name_r = r'[A-Z_]+'
    rr = re.search(name_r,line)
    if rr != None:
        name = rr[0]

        csp = "constexpr static bool {0} = static_cast<bool>({1});\n".format(name.replace("DEF_", ""),name)
    else:
        csp = ""
    return csp


def gen_constxprs(cstx,cstxbool):
    config_file_lines = []
    config_file_lines.append("\n // Build Options Structure \n struct NeMoBuildOptions {\n")
    for c in cstx:
        config_file_lines.append("\t" + c)
    for c in cstxbool:
        config_file_lines.append("\t" + c)
    config_file_lines.append("};\n inline constexpr NeMoBuildOptions nemo_build_options;\n")

    return config_file_lines


def parse_config_file(filename):

    config_file_lines = []
    config_in_file_lines = []
    constexprs = []
    constexpbools = []



    with open(filename, 'r') as f:
        in_code = False
        config_file_lines.append("""
//NOTE! This file is generated automatically using cmake from
//nemo_build_options.h.in. Changes to this file will be lost!
        """)

        for line in f:
            config_in_file_lines.append(line)

            if in_code and "#endif" in line:
                in_code = False

                for l in gen_constxprs(constexprs, constexpbools):
                    config_file_lines.append(l)
                    config_in_file_lines.append(l)


            if in_code:


                if "cmakedefine01" in line:
                    constexpbools.append(create_constbool(line))

                    line = line.replace("cmakedefine01", "define")
                    line = re.sub(r'\d.\d+', "1",line)
                    line = re.sub(r'@.+','', line)


                else:
                    constexprs.append(create_constexpr(line))
                    line = line.replace("cmakedefine","define")
                    replace = "256"
                    if "WEIGHTS" in line:
                        replace = "4"
                    elif "OUTPUTS" in line:
                        replace = "1"

                    line = re.sub( r"@.+", replace, line)





            if "#define NEMO2_NEMO_BUILD_OPTIONS_H" in line:
                in_code = True

            config_file_lines.append(line)

    return config_file_lines,config_in_file_lines


if __name__ == '__main__':
    if len(sys.argv) == 1:
        sys.argv.append(os.getcwd())
        sys.argv.append("test_out.h")

    input_filename = "nemo_build_options.h.in"
    output_filename = "nemo_build_options.h"




    print("Working directory: ")
    print(sys.argv[1])
    os.chdir(sys.argv[1])
    print("Out FN:" + sys.argv[2])
    lines,in_lines = parse_config_file(input_filename)
    with open(sys.argv[2], 'w') as f:
        f.writelines(lines)
    with open(sys.argv[2].replace(".h","_template") + ".in", 'w') as f:
        f.writelines(in_lines)

    print(os.getcwd())
    print(lines)
