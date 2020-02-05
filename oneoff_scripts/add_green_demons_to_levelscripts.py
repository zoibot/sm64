from pathlib import Path


def get_mario_pos(line):
    if "/*pos*/" not in line:
        raise Exception(f"couldn't find pos in line: {line}")
    pos_with_prefix_and_paren = line[line.index("/*pos*/") :]
    pos_with_prefix = pos_with_prefix_and_paren.strip().rstrip("),")
    return pos_with_prefix


def green_demon_object(mario_pos):
    first, second, third = mario_pos.split(", ")
    spawn_pos = ", ".join([first, str(int(second) + 1000), third])
    return (
        "    OBJECT("
        "/*model*/ MODEL_1UP, "
        f"{spawn_pos}, "
        "/*angle*/ 0, 0, 0, "
        "/*behParam*/ 0x00000000, "
        "/*beh*/ bhv1upGreenDemon),"
    )


for script in Path("./levels").glob("**/script.c"):
    response = input(f"should I do {script}?: ")
    if response.lower() != "y":
        continue

    lines = script.read_text().splitlines()
    mario_pos = None
    for line in lines:
        if "MARIO_POS(" in line:
            mario_pos = get_mario_pos(line)
            break
    if not mario_pos:
        print(f"could not find mario_pos in {script}")

    inside_script_func_local = False
    the_return_line = None
    for line in lines:
        if "script_func_local_1[]" in line:
            inside_script_func_local = True
        elif inside_script_func_local and "RETURN()" in line:
            the_return_line = line
            break
    if not the_return_line:
        print(f"could not find return line in {script}")
        continue

    object = green_demon_object(mario_pos)

    response = input(f"does this look right:\n{object}\n?: ")
    if response.lower() != "y":
        print("bailing - go figure this out")
        exit(1)

    lines.insert(lines.index(the_return_line), object)
    script.write_text("\n".join(lines) + "\n")
