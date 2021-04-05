# What are the roots that clutch, what branches grow
# Out of this stony rubbish? Son of man,
# You cannot say, or guess, for you know only
# A heap of broken images, where the sun beats,
# And the dead tree gives no shelter, the cricket no relief
#   - T.S. Eliot, The Waste Land

indent = "  "

mcd_typemap = {}
def mc_data_name(typename):
  def inner(cls):
      mcd_typemap[typename] = cls
      return cls
  return inner

# MCD/Protodef has two elements of note, "fields" and "types"
# "Fields" are JSON objects with the following members:
#   * "name" (optional): Name of the field
#   * "anonymous" (optional): Only present if Name is absent, always "True"
#   * "type": A MCD/Protodef "type"
#
# "Types" are either strings or 2-element JSON arrays
#   * String "types" are a string of the type name
#   * JSON Array "types" have the following elements:
#     * 0: String of the type name, same as String "types"
#     * 1: Type data, a JSON object who's members are type-dependent
#
# Of Note: The only MCD "type" that can contain other MCD "fields" is the
# "container" type. Even though MCD "switches" have a type data member called
# "fields" these are not MCD "fields", they are actually MCD "types".
#
# The following extract functions normalize this format

# Param: MCD "type"
# Returns: Field type, Field data
def extract_type(type_field):
  if(isinstance(type_field, str)):
    return type_field, []
  return type_field[0], type_field[1]

# Param: MCD "field"
# Returns: Field name, Field type, Field data
def extract_field(packet_field):
  name = packet_field.get('name', '')
  field_type, field_data = extract_type(packet_field['type'])
  return name, field_type, field_data

class generic_type:
  typename = ''
  postfix = ''

  def __init__(self, name, parent, type_data = None, use_compare = False):
    self.name = name
    self.old_name = None
    self.parent = parent
    self.switched = False

    # This is needed for switches because switches suck ass
    # More descriptively, switches need to be able to change the names of
    # fields when they're constructed in order to avoid name collisions. But
    # when merging later sister fields they need to be able to identify
    # duplicate fields that didn't undergo that transformation.
    #
    # To accomplish this, we repurpose the "name" field strictly as a display
    # name, and do comparisons with the "compare_name".
    self.compare_name = name
    self.use_compare = use_compare

  def declaration(self):
    return (f"{self.typename} {self.name};",)

  def initialization(self, val):
    return (f"{self.typename} {self.name} = {val};",)

  def encoder(self):
    return (f"enc_{self.postfix}(dest, {self.name});",)

  def decoder(self):
    if self.name:
      return (f"{self.name} = dec_{self.postfix}(src);",)
    # Special case for when we need to decode a variable as a parameter
    else:
      return f"dec_{self.postfix}(src)"

  def dec_initialized(self):
    return (f"{self.typename} {self.name} = dec_{self.postfix}(src);",)

  # This comes up enough to write some dedicated functions for it
  # Conglomerate types take one of two approaches to fundamental types:
  # * Set the field name _every_ time prior to decl/enc/dec
  # * Only change it when needed, and then reset it at the end of the op
  # These support the second workflow
  def temp_name(self, temp):
    if self.old_name is None:
      self.old_name = self.name
    self.name = temp

  def reset_name(self):
    if self.old_name is not None:
      self.name = self.old_name

  def __eq__(self, val):
    if not isinstance(val, type(self)):
      return False
    if hasattr(self, 'typename') and hasattr(val, 'typename'):
      if self.typename != val.typename:
        return False
    if self.use_compare:
      return self.compare_name == val.compare_name
    return self.name == val.name

  def __str__(self):
    return f"{self.typename} {self.name}"

# A "simple" type is one that doesn't need to typedef an additional type in
# order to declare its variable. The only "complex" types are bitfields and
# containers, everything else is considered "simple"
class simple_type(generic_type):
  pass

class numeric_type(simple_type):
    size = 0

# These exist because MCD switches use them. I hate MCD switches
@mc_data_name('void')
class void_type(numeric_type):
  typename = 'void'
  def declaration(self):
    return (f"// '{self.name}' is a void type",)

  def encoder(self):
    return (f"// '{self.name}' is a void type",)

  def decoder(self):
    return (f"// '{self.name}' is a void type",)

@mc_data_name('u8')
class num_u8(numeric_type):
  size = 1
  typename = 'std::uint8_t'
  postfix = 'byte'

@mc_data_name('i8')
class num_i8(num_u8):
  typename = 'std::int8_t'

@mc_data_name('bool')
class num_bool(num_u8):
  pass

@mc_data_name('u16')
class num_u16(numeric_type):
  size = 2
  typename = 'std::uint16_t'
  postfix = 'be16'

@mc_data_name('i16')
class num_i16(num_u16):
  typename = 'std::int16_t'

@mc_data_name('u32')
class num_u32(numeric_type):
  size = 4
  typename = 'std::uint32_t'
  postfix = 'be32'

@mc_data_name('i32')
class num_i32(num_u32):
  typename = 'std::int32_t'

@mc_data_name('u64')
class num_u64(numeric_type):
  size = 8
  typename = 'std::uint64_t'
  postfix = 'be64'

@mc_data_name('i64')
class num_i64(num_u64):
  typename = 'std::int64_t'

@mc_data_name('f32')
class num_float(num_u32):
  typename = 'float'
  postfix = 'bef32'

@mc_data_name('f64')
class num_double(num_u64):
  typename = 'double'
  postfix = 'bef64'

# Positions and UUIDs are broadly similar to numeric types
# A position is technically a bitfield but we hide that behind a utility func
@mc_data_name('position')
class num_position(num_u64):
  typename = 'mc_position'
  postfix = 'position'

@mc_data_name('UUID')
class num_uuid(numeric_type):
  size = 16
  typename = 'mc_uuid'
  postfix = 'uuid'

@mc_data_name('varint')
class mc_varint(numeric_type):
  # typename = 'std::int32_t'
  # All varints are varlongs until this gets fixed
  # https://github.com/PrismarineJS/minecraft-data/issues/119
  typename = 'std::int64_t'
  postfix = 'varint'

@mc_data_name('varlong')
class mc_varlong(numeric_type):
  typename = 'std::int64_t'
  # Decoding varlongs is the same as decoding varints
  postfix = 'varint'

@mc_data_name('string')
class mc_string(simple_type):
  typename = 'std::string'
  postfix = 'string'

@mc_data_name('buffer')
class mc_buffer(simple_type):
  typename = 'std::vector<char>'
  postfix = 'buffer'

  def __init__(self, name, parent, type_data, use_compare = False):
    super().__init__(name, parent, type_data, use_compare)
    self.count = mcd_typemap[type_data['countType']].postfix

  def encoder(self):
    return (
      f"enc_{self.count}(dest, {self.name}.size());",
      f"enc_buffer(dest, {self.name});",
    )

  def decoder(self):
    return (f"{self.name} = dec_buffer(src, dec_{self.count}(src));",)

@mc_data_name('restBuffer')
class mc_restbuffer(simple_type):
  typename = 'std::vector<char>'
  postfix = 'buffer'

  def encoder(self):
    return (f"dest.write({self.name}.data(), {self.name}.size());",)

  def decoder(self):
    return (
      f"{self.name} = std::vector<char>(std::istreambuf_iterator<char>(src),",
      indent*2 + f"std::istreambuf_iterator<char>());"
    )

@mc_data_name('nbt')
class mc_nbt(simple_type):
  typename = 'nbt::TagCompound'

  def encoder(self):
    return (f"{self.name}.encode_full(dest);",)

  def decoder(self):
    return (f"{self.name}.decode_full(src);",)

@mc_data_name('optionalNbt')
class mc_optnbt(simple_type):
  typename = 'std::optional<nbt::TagCompound>'

  def encoder(self):
    return (
      f"if({self.name})",
      f"{indent}{self.name}->encode_full(dest);",
      f"else",
      f"{indent}enc_byte(dest, nbt::TAG_END);"
    )

  def decoder(self):
    return (
      "if(dec_byte(src) == nbt::TAG_COMPOUND)",
      f"{indent}{self.name}.emplace(src, nbt::read_string(src));",
    )

class self_serializing_type(simple_type):
  def encoder(self):
    return (f"{self.name}.encode(dest);",)

  def decoder(self):
    return (f"{self.name}.decode(src);",)

@mc_data_name('slot')
class mc_slot(self_serializing_type):
  typename = 'MCSlot'

@mc_data_name('minecraft_smelting_format')
class mc_smelting(self_serializing_type):
  typename = 'MCSmelting'

@mc_data_name('entityMetadata')
class mc_metadata(self_serializing_type):
  typename = 'MCEntityMetadata'

# This is not how topBitSetTerminatedArray works, but the real solution is hard
# and this solution is easy. As long as this type is only found in the Entity
# Equipment packet we're going to stick with this solution
@mc_data_name('topBitSetTerminatedArray')
class mc_entity_equipment(self_serializing_type):
  typename = 'MCEntityEquipment'

@mc_data_name('particleData')
class mc_particle(self_serializing_type):
  typename = 'MCParticle'
  def __init__(self, name, parent, type_data, use_compare = False):
    super().__init__(name, parent, type_data, use_compare)
    self.id_field = type_data['compareTo']

  def decoder(self):
    return (f"{self.name}.decode(src, "
        f"static_cast<particle_type>({self.id_field}));",)

class vector_type(simple_type):
  count = mc_varint
  def __init__(self, name, parent, type_data, use_compare = False):
    super().__init__(name, parent, type_data, use_compare)
    self.depth = 0
    p = parent
    while not isinstance(p, packet):
      self.depth += 1
      p = p.parent

  def encoder(self):
    return (
      f"enc_{self.count.postfix}(dest, {self.name}.size());",
      f"for(auto &el{self.depth} : {self.name})",
      f"{indent}el{self.depth}.encode(dest);"
    )

  def decoder(self):
    return (
      f"{self.name}.resize(dec_{self.count.postfix}(src));",
      f"for(auto &el{self.depth} : {self.name})",
      f"{indent}el{self.depth}.decode(src);"
    )

@mc_data_name('ingredient')
class mc_ingredient(vector_type):
  typename = 'std::vector<MCSlot>'

@mc_data_name('tags')
class mc_tags(vector_type):
  typename = 'std::vector<MCTag>'

@mc_data_name('option')
class mc_option(simple_type):
  def __init__(self, name, parent, type_data, use_compare = False):
    super().__init__(name, parent, type_data, use_compare)
    f_type, f_data = extract_type(type_data)
    self.field = mcd_typemap[f_type](f"{name}", self, f_data)

    if isinstance(self.field, simple_type):
      self.typename = f"std::optional<{self.field.typename}>"
    else:
      self.typename = f"std::optional<{self.name}_type>"

  def declaration(self):
    if isinstance(self.field, simple_type):
      return super().declaration()
    self.field.name = f"{self.name}_type"
    return [
      *self.field.typedef(),
      f"{self.typename} {self.name};"
    ]

  def encoder(self):
    self.field.name = f"{self.name}.value()"
    return [
      f"enc_byte(dest, {self.name}.has_value());",
      f"if({self.name}) {{",
      *(indent + line for line in self.field.encoder()),
      "}"
    ]

  def decoder(self):
    ret = [f"if(dec_byte(src)) {{"]
    if isinstance(self.field, numeric_type) or type(self.field) in (mc_string,
        mc_buffer, mc_restbuffer):
      self.field.name = self.name
    elif isinstance(self.field, vector_type) or isinstance(self.field,
        complex_type):
      ret.append(f"{indent}{self.name}.emplace();")
      self.field.name = f"{self.name}.value()"
    else:
      self.field.name = f"{self.name}.emplace()"
    ret.extend(indent + line for line in self.field.decoder())
    ret.append('}')
    return ret

class complex_type(generic_type):
  def declaration(self):
    if self.name:
      return [
        "struct {",
        *(indent + l for f in self.fields for l in f.declaration()),
        f"}} {self.name};"
      ]
    return [l for f in self.fields for l in f.declaration()]

  def typedef(self):
    ret = self.declaration()
    ret[0] = f"typedef {ret[0]}"
    return ret

def get_storage(numbits):
  if numbits <= 8:
    return 8
  elif numbits <= 16:
    return 16
  elif numbits <= 32:
    return 32
  else:
    return 64

@mc_data_name('bitfield')
class mc_bitfield(complex_type):
  def __init__(self, name, parent, type_data, use_compare = False):
    super().__init__(name, parent, type_data, use_compare)
    lookup_unsigned = {
        8: num_u8,
        16: num_u16,
        32: num_u32,
        64: num_u64
    }
    lookup_signed = {
        8: num_i8,
        16: num_i16,
        32: num_i32,
        64: num_i64
    }
    self.fields = []
    self.extra_data = []
    self.field_sizes = {}

    total = 0
    for idx, field in enumerate(type_data):
      total += field['size']
      if field['name'] in ('_unused', 'unused'):
        continue
      self.field_sizes[field['name']] = field['size']
      shift = 0
      for temp in type_data[idx + 1:]:
        shift += temp['size']
      self.extra_data.append(((1<<field['size'])-1, shift, field['size'],
          field['signed']))
      numbits = get_storage(field['size'])
      if field['signed']:
        self.fields.append(lookup_signed[numbits](field['name'], self))
      else:
        self.fields.append(lookup_unsigned[numbits](field['name'], self))

    self.storage = lookup_unsigned[total](f"{name}_", self)
    self.size = total//8

  def encoder(self):
    ret = [*self.storage.initialization("0")]
    name = f"{self.name}." if self.name else ""
    for idx, field in enumerate(self.fields):
      mask, shift, size, signed = self.extra_data[idx]
      shift_str = f"<<{shift}" if shift else ""
      ret.append(f"{self.storage.name} |= "
          f"(static_cast<{self.storage.typename}>({name}"
          f"{field.name})&{mask}){shift_str};")
    ret.extend(self.storage.encoder())
    return ret

  def decoder(self):
    ret = [*self.storage.dec_initialized()]
    name = f"{self.name}." if self.name else ""
    for idx, field in enumerate(self.fields):
      mask, shift, size, signed = self.extra_data[idx]
      shift_str = f">>{shift}" if shift else ""
      ret.append(f"{name}{field.name} = "
          f"({self.storage.name}{shift_str})&{mask};")
      if signed:
        ret.extend((
          f"if({name}{field.name} & (1UL << {size - 1}))",
          f"{indent}{name}{field.name} -= 1UL << {size};"
        ))
    return ret

# Whatever you think an MCD "switch" is you're probably wrong
@mc_data_name('switch')
class mc_switch(simple_type):
  def __init__(self, name, parent, type_data, use_compare = False):
    super().__init__(name, parent, type_data, use_compare)
    self.compareTo = type_data['compareTo']
    # Unions are a sum type that differs based on conditions. They are a true
    # "switch". All others are here because MCData is bad and I hate it.
    self.is_union = False

    # A non-union switch consists of one or more consistent types, some or all
    # of which are decoded under different conditions. They act like unions
    # except they leave behind Null switches if they're multi-element.
    #
    # Null switches are fields that got merged into a sister switch, they're
    # carcasses that don't do anything. They're found when MCD encodes a
    # multi-element switch. All the elements except the "lead" element become
    # null switches.
    self.null_switch = False
    # Inverses are default present _except_ for certain conditions, and they're
    # bugged on tons of corner cases that thankfully aren't present in MCD
    # right now.
    self.is_inverse = False
    self.is_str_switch = False

    self.lead_sister = None
    self.fields = []
    self.field_dict = {}

    # Find out if all possible values are the same type, in which case we're
    # not a union. And if all the types are void we're an inverse.
    values = type_data['fields'].values()
    if(all(map(lambda x : x == 'void', values))):
      self.is_inverse = True
      f_type, f_data = extract_type(type_data['default'])
      self.fields.append(mcd_typemap[f_type](name, self, f_data))
    else:
      values = list(filter(lambda x : x != 'void', values))
      self.is_union = not all(x == values[0] for x in values)

    # In the case we're not a union, we need to check for sister switches
    if not self.is_union:
      for field in parent.fields:
        if isinstance(field, mc_switch) and field.compareTo == self.compareTo:
          self.null_switch = True
          self.lead_sister = field.lead_sister if field.null_switch else field
          self.lead_sister.merge(name, type_data['fields'], self.is_inverse,
              type_data.get('default', None))
          return

    self.process_fields(self.name, type_data['fields'])

  def declaration(self):
    if self.null_switch:
      return []
    return [l for f in self.fields for l in f.declaration()]

  def code_fields(self, ret, fields, encode = True):
    for field in fields:
      if hasattr(self.parent, 'name') and self.parent.name:
        field.temp_name(f"{self.parent.name}.{field.name}")
      if encode:
        ret.extend(f"{indent}{l}" for l in field.encoder())
      else:
        ret.extend(f"{indent}{l}" for l in field.decoder())
      field.reset_name()
    return ret

  def inverse(self, comp, encode = True):
    if len(self.field_dict.items()) == 1:
      case, _ = next(iter(self.field_dict.items()))
      ret = [f"if({comp} != {case}) {{"]
    else:
      return ('// Multi-Condition Inverse Not Yet Implemented',)
    self.code_fields(ret, self.fields, encode)
    ret.append("}")
    return ret

  # Special case for single condition switches, which are just optionals
  # mascarading as switches
  def optional(self, comp, encode = True):
    case, fields = next(iter(self.field_dict.items()))
    ret = []
    if case == "true":
      ret.append(f"if({comp}) {{")
    elif case == "false":
      ret.append(f"if(!{comp}) {{")
    elif case.isdigit():
      ret.append(f"if({comp} == {case}) {{")
    else:
      ret.append(f"if(!{comp}.compare({case})) {{")
    self.code_fields(ret, fields, encode)
    ret.append("}")
    return ret

  def str_switch(self, comp, encode = True):
    items = list(self.field_dict.items())
    case, fields = items[0]
    ret = [f"if(!{comp}.compare({case})) {{"]
    self.code_fields(ret, fields, encode)
    for case, fields in items[1:]:
      ret.append(f"}} else if(!{comp}.compare({case})) {{")
      self.code_fields(ret, fields, encode)
    ret.append("}")
    return ret

  def union_multi(self, comp, encode = True):
    if len(self.field_dict.items()) == 1:
      return self.optional(comp, encode)
    ret = [f"switch({comp}) {{"]
    for case, fields in self.field_dict.items():
      ret.append(f"{indent}case {case}:")
      tmp = []
      self.code_fields(tmp, fields, encode)
      ret.extend(indent + l for l in tmp)
      ret.append(f"{indent*2}break;")
    ret.append("}")
    return ret

  # compareTo is a mystical format, its powers unmatched. We don't try to parse
  # it. Instead, we just count the number of hierarchy levels it ascends with
  # ".." and move up the container hierarchy using that. If we hit the packet,
  # we abandon ship and assume the path is absolute.
  def get_compare(self):
    comp = self.compareTo.replace('../','').replace('..','').replace('/','.')
    p = self.parent
    for i in range(self.compareTo.count('..')):
      while not (isinstance(p, complex_type) or isinstance(p, packet)):
        p = p.parent
      if isinstance(p, packet):
        break;
      else:
        p = p.parent
    while not (isinstance(p, complex_type) or isinstance(p, packet)):
      p = p.parent
    if not isinstance(p, packet):
      comp = f"{p.name}.{comp}"
    return comp

  def encoder(self):
    comp = self.get_compare()
    if self.null_switch:
      return []
    if self.is_inverse:
      return self.inverse(comp)
    if self.is_str_switch:
      return self.str_switch(comp)
    return self.union_multi(comp)

  def decoder(self):
    comp = self.get_compare()
    if self.null_switch:
      return []
    if self.is_inverse:
      return self.inverse(comp, False)
    if self.is_str_switch:
      return self.str_switch(comp, False)
    return self.union_multi(comp, False)

  def process_fields(self, name, fields):
    for key, field_info in fields.items():
      if not key.isdigit() and key not in ("true", "false"):
        key = f'"{key}"'
        self.is_str_switch = True
      f_type, f_data = extract_type(field_info)
      field = mcd_typemap[f_type](name, self, f_data, True)
      if field not in self.fields:
        if not isinstance(field, void_type):
          self.fields.append(field)
      if not self.is_inverse and isinstance(field, void_type):
        continue
      if key in self.field_dict:
        self.field_dict[key].append(field)
      else:
        self.field_dict[key] = [field]

      # Look for name collisions and fix them. This is unnecessarily convoluted
      # and ripe for refactoring.
      has_dupes = []
      for field in self.fields:
        if len(list(filter(lambda x: x.compare_name == field.compare_name,
            self.fields))) > 1:
          has_dupes.append(field)
      # If we're an anonymous switch don't bother, we'll break container fields
      if has_dupes and name:
        for key, fields in self.field_dict.items():
          # Is this fine? Is it guaranteed there will be (at least) one and
          # only one dupe for each key? I think so?
          same = next(x for x in fields if x in has_dupes)
          if self.is_str_switch:
            tmp = key.replace('"', '').replace(':', '_')
            same.name = tmp
            # As a weird consequence of the merging strategy, we can end up in
            # a situation with two tags that compare the same but have
            # different names. These tags have the same type, but because
            # string switch fields have "convenient" names they have different
            # names. Here we restore these "lost" tags to the field list
            if all(map(lambda x: x.name != same.name, self.fields)):
              self.fields.append(same)
          else:
            same.name = f"{name}_{key}"

      if not self.is_str_switch:
        self.field_dict = {key: self.field_dict[key] for key in
            sorted(self.field_dict)}

  def merge(self, sis_name, sis_fields, is_inverse, default):
    if is_inverse:
      f_type, f_data = extract_type(default)
      self.fields.append(mcd_typemap[f_type](sis_name, self, f_data))
    self.process_fields(sis_name, sis_fields)

  def __eq__(self, value):
    if not super().__eq__(value) or len(self.fields) != len(value.fields):
        return False
    return all([i == j for i, j in zip(self.fields, value.fields)])


# Arrays come in three flavors. Fixed length, length prefixed, and foreign
# field
@mc_data_name('array')
class mc_array(simple_type):
  def __init__(self, name, parent, type_data, use_compare = False):
    super().__init__(name, parent, type_data, use_compare)
    f_type, f_data = extract_type(type_data['type'])
    self.field = mcd_typemap[f_type]('', self, f_data)
    self.depth = 0
    p = parent
    while not isinstance(p, packet):
      self.depth += 1
      p = p.parent
    self.packet = p

    self.is_fixed = False
    self.is_prefixed = False
    self.is_foreign = False
    self.count = None
    if "countType" in type_data:
      self.is_prefixed = True
      self.count = mcd_typemap[type_data['countType']]('', self, [])
    elif isinstance(type_data['count'], int):
      self.is_fixed = True
      self.count = type_data['count']
    else:
      self.is_foreign = True
      self.count = type_data['count']

    if isinstance(self.field, simple_type):
      self.typename = f"std::vector<{self.field.typename}>"
    else:
      self.typename = f"std::vector<{self.field.name}>"

  def declaration(self):
    if isinstance(self.field, simple_type):
      ret = []
      f_type = self.field.typename
    else:
      self.field.name = f"{self.packet.packet_name}_{self.name}_type"
      ret = self.field.typedef()
      f_type = self.field.name
    if self.is_fixed:
      ret.append(f"std::array<{f_type}, {self.count}> {self.name};")
    else:
      ret.append(f"std::vector<{f_type}> {self.name};")
    return ret

  def fixed(self, encode = True):
    self.field.name = f"el{self.depth}"
    ret = [f"for(auto &el{self.depth} : {self.name}) {{"]
    if encode:
      ret.extend(indent + l for l in self.field.encoder())
    else:
      ret.extend(indent + l for l in self.field.decoder())
    ret.append("}")
    return ret

  def prefixed_encode(self):
    self.count.name = f"{self.name}.size()"
    self.field.name = f"el{self.depth}"
    return [
      *self.count.encoder(),
      f"for(auto &el{self.depth} : {self.name}) {{",
      *(indent + l for l in self.field.encoder()),
      "}"
    ]

  def prefixed_decode(self):
    self.count.name = ''
    self.field.name = f"el{self.depth}"
    return [
      f"{self.name}.resize({self.count.decoder()});",
      f"for(auto &el{self.depth} : {self.name}) {{",
      *(indent + l for l in self.field.decoder()),
      "}"
    ]

  # Identical to switches' compareTo
  def get_foreign(self):
    comp = self.count.replace('../','').replace('..','').replace('/','.')
    p = self.parent
    for i in range(self.count.count('..')):
      while not (isinstance(p, complex_type) or isinstance(p, packet)):
        p = p.parent
      if isinstance(p, packet):
        break;
      else:
        p = p.parent
    while not (isinstance(p, complex_type) or isinstance(p, packet)):
      p = p.parent
    if not isinstance(p, packet):
      comp = f"{p.name}.{comp}"
    return comp

  def foreign_encode(self):
    self.field.name = f"el{self.depth}"
    return [
      f"for(auto &el{self.depth} : {self.name}) {{",
      *(indent + l for l in self.field.encoder()),
      "}"
    ]

  def foreign_decode(self):
    self.field.name = f"el{self.depth}"
    return [
      f"{self.name}.resize({self.get_foreign()});",
      f"for(auto &el{self.depth} : {self.name}) {{",
      *(indent + l for l in self.field.decoder()),
      "}"
    ]

  def encoder(self):
    if self.is_fixed:
      return self.fixed()
    if self.is_prefixed:
      return self.prefixed_encode()
    return self.foreign_encode()

  def decoder(self):
    if self.is_fixed:
      return self.fixed(False)
    if self.is_prefixed:
      return self.prefixed_decode()
    return self.foreign_decode()

# Not a container_type because containers_types sometimes have trivial storage
# requirements. Actual containers always have non-trivial storage, which makes
# them a pure complex type
@mc_data_name('container')
class mc_container(complex_type):
  def __init__(self, name, parent, type_data, use_compare = False):
    super().__init__(name, parent, type_data, use_compare)

    self.fields = []
    for field_info in type_data:
      f_name, f_type, f_data = extract_field(field_info)
      self.fields.append(mcd_typemap[f_type](f_name, self, f_data))

  def encoder(self):
    ret = []
    if self.name:
      for field in self.fields:
        field.temp_name(f"{self.name}.{field.name}")
        ret.extend(field.encoder())
        field.reset_name()
    else:
      for field in self.fields:
        ret.extend(field.encoder())
    return ret

  def decoder(self):
    ret = []
    if self.name:
      for field in self.fields:
        field.temp_name(f"{self.name}.{field.name}")
        ret.extend(field.decoder())
        field.reset_name()
    else:
      for field in self.fields:
        ret.extend(field.decoder())
    return ret

  def __eq__(self, value):
    if not super().__eq__(value) or len(self.fields) != len(value.fields):
        return False
    return all([i == j for i, j in zip(self.fields, value.fields)])


class packet:
  def __init__(self, state, direction, packet_id, packet_name, data):
    self.state = state
    self.direction = "Clientbound" if direction == "toClient" else "Serverbound"
    self.packet_id = packet_id
    self.packet_name = packet_name
    self.data = data
    self.class_name = f"{self.direction}{self.packet_name}"

    self.fields = []
    for data_field in data:
      f_name, f_type, f_data = extract_field(data_field)
      self.fields.append(mcd_typemap[f_type](f_name, self, f_data))

  def declaration(self):
    return [
      f"class {self.class_name} : public Packet {{",
      f"public:",
      *(indent + l for f in self.fields for l in f.declaration()),
      f"{indent}{self.class_name}();",
      f"{indent}void encode(std::ostream &dest) const;",
      f"{indent}void decode(std::istream &src);",
      "};"
    ]

  def constructor(self):
    return [
      f"{self.class_name}::{self.class_name}() :",
      f"{indent*2}Packet({self.state.upper()}, {self.direction.upper()}, "
          f"{self.packet_id},",
      f"{indent*2}\"{self.class_name}\") {{}}"
    ]

  def encoder(self):
    return [
      f"void {self.class_name}::encode(std::ostream &dest) const {{",
      *(indent + l for f in self.fields for l in f.encoder()),
      "}"
    ]

  def decoder(self):
    return [
      f"void {self.class_name}::decode(std::istream &src) {{",
      *(indent + l for f in self.fields for l in f.decoder()),
      "}"
    ]

mc_states = "handshaking", "status", "login", "play"
mc_directions = "toClient", "toServer"

warning = (
  "/*",
  "  This file was generated by mcd2cpp.py",
  "  It should not be edited by hand.",
  "*/",
  "",
)

import re

first_cap_re = re.compile('(.)([A-Z][a-z]+)')
all_cap_re = re.compile('([a-z0-9])([A-Z])')
def to_enum(name, direction, state):
    s1 = first_cap_re.sub(r'\1_\2', name)
    name = all_cap_re.sub(r'\1_\2', s1).upper()
    d = "SB" if direction == "toServer" else "CB"
    st = {"handshaking": "HS", "status": "ST", "login": "LG",
        "play": "PL"}[state]
    return f"{d}_{st}_{name}"

def to_camel_case(string):
  return string.title().replace('_', '')

def extract_infos_from_listing(listing):
  ret = []
  # Why this seemingly random position inside the full listing? Because mcdata
  # hates you.
  for p_id, p_name in listing['types']['packet'][1][0]['type'][1]['mappings'].items():
    ret.append((int(p_id, 0), to_camel_case(p_name), f"packet_{p_name}"))
  return ret

def run(mcd):
  version = mcd.version['minecraftVersion'].replace('.', '_')
  proto = mcd.protocol
  header_upper = [
    *warning,
    f"#ifndef PROTO_{version}_HPP",
    f"#define PROTO_{version}_HPP",
    "",
    "#include <cstdint>",
    "#include <string>",
    "#include <vector>",
    "#include <array>",
    "#include <optional>",
    "#include <memory>",
    "#include <stdexcept>",
    "#include \"datautils.hpp\"",
    "#include \"nbt.hpp\"",
    "",
    f"#define MC_PROTO_VERSION {mcd.version['version']}",
    "",
    "namespace mcd {",
    "",
    "enum packet_direction {",
    "  SERVERBOUND,",
    "  CLIENTBOUND,",
    "  DIRECTION_MAX",
    "};",
    "",
    "enum packet_state {",
    "  HANDSHAKING,",
    "  STATUS,",
    "  LOGIN,",
    "  PLAY,",
    "  STATE_MAX",
    "};",
    ""
  ]
  header_lower = [
    "extern const char* serverbound_handshaking_cstrings["
      "SERVERBOUND_HANDSHAKING_MAX];",
    "extern const char* clientbound_status_cstrings[CLIENTBOUND_STATUS_MAX];",
    "extern const char* serverbound_status_cstrings[SERVERBOUND_STATUS_MAX];",
    "extern const char* clientbound_login_cstrings[CLIENTBOUND_LOGIN_MAX];",
    "extern const char* serverbound_login_cstrings[SERVERBOUND_LOGIN_MAX];",
    "extern const char* clientbound_play_cstrings[CLIENTBOUND_PLAY_MAX];",
    "extern const char* serverbound_play_cstrings[SERVERBOUND_PLAY_MAX];",
    "",
    "extern const char **protocol_cstrings[STATE_MAX][DIRECTION_MAX];",
    "extern const int protocol_max_ids[STATE_MAX][DIRECTION_MAX];",
    "",
    "class Packet {",
    "public:",
    "  const packet_state state;",
    "  const packet_direction direction;",
    "  const std::int32_t packet_id;",
    "  const std::string packet_name;",
    "",
    "  Packet(packet_state state, packet_direction direction,",
    "      std::int32_t packet_id, std::string name) : state(state), ",
    "      direction(direction), packet_id(packet_id), packet_name(name) {}",
    "  virtual ~Packet() = default;",
    "  virtual void encode(std::ostream &buf) const = 0;",
    "  virtual void decode(std::istream &buf) = 0;",
    "};",
    "",
    "std::unique_ptr<Packet> make_packet(packet_state state, "
    "packet_direction dir,",
    f"    int packet_id);",
    ""
  ]
  impl_upper = [
    *warning,
    f"#include \"proto_{version}.hpp\"",
    "",
    "namespace mcd {",
    ""
  ]
  impl_lower = []
  packet_enum = {}
  packets = {}

  for state in mc_states:
    packet_enum[state] = {}
    packets[state] = {}
    for direction in mc_directions:
      packet_enum[state][direction] = []
      packets[state][direction] = []
      packet_infos = extract_infos_from_listing(proto[state][direction])
      for info in packet_infos:
        packet_data = proto[state][direction]['types'][info[2]][1]
        if info[1] != "LegacyServerListPing":
          packet_id = to_enum(info[1], direction, state)
          packet_enum[state][direction].append(packet_id)
        else:
          packet_id = info[0]
        pak = packet(state, direction, packet_id, info[1], packet_data)
        if info[1] != "LegacyServerListPing":
          packets[state][direction].append(pak)
        header_lower += pak.declaration()
        header_lower.append('')

        impl_lower += pak.constructor()
        impl_lower += pak.encoder()
        impl_lower += pak.decoder()
        impl_lower.append('')

  for state in mc_states:
    for direction in mc_directions:
      dr = "clientbound" if direction == "toClient" else "serverbound"
      packet_enum[state][direction].append(f"{dr.upper()}_{state.upper()}_MAX")
      header_upper.append(f"enum {dr}_{state}_ids {{")
      header_upper.extend(f"{indent}{l}," for l in packet_enum[state][direction])
      header_upper[-1] = header_upper[-1][:-1]
      header_upper.extend(("};", ""))

  make_packet = [
    "std::unique_ptr<Packet> make_packet(packet_state state, "
    "packet_direction dir,",
    f"    int packet_id) {{",
    "  switch(state) {"
  ]

  for state in mc_states:
    make_packet.append(f"{indent*2}case {state.upper()}:")
    make_packet.append(f"{indent*3}switch(dir) {{")
    for direction in mc_directions:
      dr = "CLIENTBOUND" if direction == "toClient" else "SERVERBOUND"
      make_packet.append(f"{indent*4}case {dr}:")
      make_packet.append(f"{indent*5}switch(packet_id) {{")
      for pak in packets[state][direction]:
        make_packet.append(f"{indent*6} case {pak.packet_id}:")
        make_packet.append(f"{indent*7} return "
            f"std::make_unique<{pak.class_name}>();")
      make_packet.append(f"{indent*6}default:")
      make_packet.append(f"{indent*7}throw std::runtime_error(\"Invalid "
          "Packet Id\");")
      make_packet.append(f"{indent*5}}}")
    make_packet.append(f"{indent*4}default:")
    make_packet.append(f"{indent*5}throw std::runtime_error(\"Invalid "
        "Packet Direction\");")
    make_packet.append(f"{indent*3}}}")
  make_packet.append(f"{indent*2}default:")
  make_packet.append(f"{indent*3}throw std::runtime_error(\"Invalid "
    "Packet State\");")
  make_packet.extend((f"{indent}}}", "}", ""))

  for state in mc_states:
    for direction in mc_directions:
      if packets[state][direction]:
        dr = "clientbound" if direction == "toClient" else "serverbound"
        impl_upper.append(f"const char* {dr}_{state}_cstrings[] = {{")
        for pak in packets[state][direction]:
          impl_upper.append(f"{indent}\"{pak.class_name}\",")
        impl_upper[-1] = impl_upper[-1][:-1]
        impl_upper.extend(("};", ""))

  impl_upper += [
    "const char **protocol_cstrings[STATE_MAX][DIRECTION_MAX] = {",
    f"{indent}{{serverbound_handshaking_cstrings}},",
    f"{indent}{{serverbound_status_cstrings, clientbound_status_cstrings}},",
    f"{indent}{{serverbound_login_cstrings, clientbound_login_cstrings}},",
    f"{indent}{{serverbound_play_cstrings, clientbound_play_cstrings}}",
    "};",
    "",
    "const int protocol_max_ids[STATE_MAX][DIRECTION_MAX] = {",
    f"{indent}{{SERVERBOUND_HANDSHAKING_MAX, CLIENTBOUND_HANDSHAKING_MAX}},",
    f"{indent}{{SERVERBOUND_STATUS_MAX, CLIENTBOUND_STATUS_MAX}},",
    f"{indent}{{SERVERBOUND_LOGIN_MAX, CLIENTBOUND_LOGIN_MAX}},",
    f"{indent}{{SERVERBOUND_PLAY_MAX, CLIENTBOUND_PLAY_MAX}}",
    "};",
    "",
  ]

  header = header_upper + header_lower + ["}", "#endif", ""]
  impl = impl_upper + impl_lower + make_packet + ["}", ""]


  with open(f"proto_{version}.cpp", "w") as f:
    f.write('\n'.join(impl))
  with open(f"proto_{version}.hpp", "w") as f:
    f.write('\n'.join(header))

if __name__ == "__main__":
  run("1.16.1")
