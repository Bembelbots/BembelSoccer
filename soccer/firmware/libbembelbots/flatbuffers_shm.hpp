#pragma once

#include <flatbuffers/flatbuffer_builder.h>

template<typename FB>
class BembelbotsShmFlatbuffer {
public:
    using NativeTableType = typename FB::NativeTableType;

    // ensure buffer always contains valid flatbuffer string
    BembelbotsShmFlatbuffer() { set(); }

    bool set(const NativeTableType &sm = {}) {
        // pack flatbuffer
        flatbuffers::FlatBufferBuilder builder;
        auto packed = FB::Pack(builder, &sm);
        builder.FinishSizePrefixed(packed);

        // sanity check
        if (builder.GetSize() > BB_SHM_BUF_SIZE)
            return false;

        // copy to SHM
        std::memcpy(data, builder.GetBufferPointer(), builder.GetSize());
        return true;
    }

    bool get(NativeTableType &fb) const {
        flatbuffers::Verifier v(data, BB_SHM_BUF_SIZE);
        if (!v.VerifySizePrefixedBuffer<FB>(nullptr))
            return false;

        auto *root{::flatbuffers::GetSizePrefixedRoot<FB>(data)};
        root->UnPackTo(&fb);

        return true;
    }

private:
    static constexpr size_t BB_SHM_BUF_SIZE{2048};
    uint8_t data[BB_SHM_BUF_SIZE];
};
