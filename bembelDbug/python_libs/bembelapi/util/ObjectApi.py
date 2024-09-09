import flatbuffers


def camelCase(name):
    import re
    name = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', name)
    name = re.sub('([a-z0-9])([A-Z])', r'\1_\2', name).lower()
    name = ''.join(x.capitalize() or '_' for x in name.split('_'))
    name = name[0].lower() + name[1:]
    return name


def bembelbuf(buffer_namespace):

    buffer_name = buffer_namespace.__name__.split('.')[-1]
    buffer_class = getattr(buffer_namespace, buffer_name)

    def create_buffer_function(self, name, value):
        new_name = name.replace(buffer_name, "")
        setattr(self, "_" + camelCase(new_name), value)

    def decorate(c):
        old_init = c.__init__

        for attr in dir(buffer_namespace):
            if attr == buffer_name or attr.startswith("_"):
                continue

            fn = getattr(buffer_namespace, attr)

            def fn_wrapper(fn):
                def wrap(self, *args, **kwargs):
                    return fn(*args, **kwargs)
                return wrap

            create_buffer_function(c, attr, fn_wrapper(fn))

        c._buffer_attrs = []
        c._buffer_attrs_map = {}
        for attr in dir(buffer_class):
            if attr.startswith("_"):
                continue

            if attr.startswith('GetRoot') or attr.startswith('Init'):
                # ignore
                continue

            setattr(c, camelCase(attr), None)
            c._buffer_attrs.append(camelCase(attr))
            c._buffer_attrs_map[attr] = c._buffer_attrs[-1]

        def new_init(self, *args, **kwargs):
            old_init(self, *args, **kwargs)

        def new_iter(self):
            for attr in self._buffer_attrs:
                yield attr, getattr(self, attr)

        def get_name(self):
            return buffer_name

        @classmethod
        def buffer_init(cls, buf, offset):
            base_inst = buffer_class()
            base_inst.Init(buf, offset)
            inst = c()
            for fn_name, attr in c._buffer_attrs_map.items():
                try:
                    val = getattr(buffer_class, fn_name)(base_inst)
                    if isinstance(val, bytes):
                        val = val.decode('utf-8')
                        #print(val, fn_name)
                        #val = "" if (val is None) else val
                        #print(val, fn_name)
                    setattr(inst, attr, val)
                except Exception as e:
                    pass
            return inst

        @classmethod
        def as_root(cls, buf, offset):
            n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
            return c.init(buf, n + offset)

        c.__init__ = new_init
        c.__iter__ = new_iter
        c.getName = get_name
        c.init = buffer_init
        c.asRoot = as_root

        return c

    return decorate


def SerializeBuffer(inst):
    builder = flatbuffers.Builder(256)
    s = inst.serialize(builder)
    builder.FinishSizePrefixed(s)
    return builder.Output()
