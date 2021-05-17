def run(mcd, indent = '  '):
  i = indent
  shape_hdr = [
    "/*",
    f"{i}This file was generated by mcd2cpp.py",
    f"{i}It should not be edited by hand.",
    "*/",
    "",
    f"// For MCD version {mcd.version['minecraftVersion']}",
    "",
    "#ifndef SHAPEDATA_HPP",
    "#define SHAPEDATA_HPP",
    "",
    "#include <vector>",
    "",
    "namespace mcd {",
    "",
    "union MCPoint {",
    f"{i}struct {{",
    f"{i*2}double x, y, z;",
    f"{i}}};",
    f"{i}double vec[3];",
    "};",
    "",
    "struct MCBBoxData {",
    f"{i}MCPoint min, max;",
    "};",
    "",
    f"extern const std::vector<MCBBoxData> shapes[{len(mcd.blockCollisionShapes['shapes'])}];",
    "",
    "} // namespace mcd",
    "#endif // SHAPEDATA_HPP",
  ]

  with open(f"shape_data.hpp", "w") as f:
    f.write('\n'.join(shape_hdr))

  shape_cpp = [
    "/*",
    f"{i}This file was generated by mcd2cpp.py",
    f"{i}It should not be edited by hand.",
    "*/",
    "",
    "#include \"shape_data.hpp\"",
    "",
    "namespace mcd {",
    ""
    "const std::vector<MCBBoxData> shapes[] = {",
  ]

  for shape in mcd.blockCollisionShapes['shapes'].values():
    shape_cpp.append(f"{i}{{{'' if shape else '},'}")
    if shape:
      shape_cpp.append(',\n'.join(
          f"{i*2}{{\n"
          f"{i*3}.min = {{{box[0]}, {box[1]}, {box[2]}}},\n"
          f"{i*3}.max = {{{box[3]}, {box[4]}, {box[5]}}},\n"
          f"{i*2}}}"
      for box in shape))
      shape_cpp.append(f"{i}}},")
  shape_cpp[-1] = shape_cpp[-1][:-1]

  shape_cpp.extend((
    "};",
    "",
    "} // namespace mcd",
  ))

  with open(f"shape_data.cpp", "w") as f:
    f.write('\n'.join(shape_cpp))