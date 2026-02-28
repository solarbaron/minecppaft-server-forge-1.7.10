/**
 * NBT.cpp — NBT implementation (non-inline methods).
 *
 * Java reference: net.minecraft.nbt.NBTBase and all subclasses.
 */

#include "nbt/NBT.h"

namespace mccpp {
namespace nbt {

// ─── NBTBase factory ────────────────────────────────────────────────────────

std::unique_ptr<NBTBase> NBTBase::createNewByType(TagType type) {
    // Java reference: NBTBase.createNewByType(byte)
    switch (type) {
        case TagType::End:       return std::make_unique<NBTTagEnd>();
        case TagType::Byte:      return std::make_unique<NBTTagByte>();
        case TagType::Short:     return std::make_unique<NBTTagShort>();
        case TagType::Int:       return std::make_unique<NBTTagInt>();
        case TagType::Long:      return std::make_unique<NBTTagLong>();
        case TagType::Float:     return std::make_unique<NBTTagFloat>();
        case TagType::Double:    return std::make_unique<NBTTagDouble>();
        case TagType::ByteArray: return std::make_unique<NBTTagByteArray>();
        case TagType::String:    return std::make_unique<NBTTagString>();
        case TagType::List:      return std::make_unique<NBTTagList>();
        case TagType::Compound:  return std::make_unique<NBTTagCompound>();
        case TagType::IntArray:  return std::make_unique<NBTTagIntArray>();
    }
    return nullptr;
}

const char* NBTBase::getTypeName(TagType type) {
    // Java reference: NBTBase.NBT_TYPES[]
    static const char* names[] = {
        "END", "BYTE", "SHORT", "INT", "LONG",
        "FLOAT", "DOUBLE", "BYTE[]", "STRING",
        "LIST", "COMPOUND", "INT[]"
    };
    int id = static_cast<int>(type);
    if (id >= 0 && id <= 11) return names[id];
    return "UNKNOWN";
}

// ─── NBTTagList accessors ───────────────────────────────────────────────────

NBTTagCompound* NBTTagList::getCompoundTagAt(int32_t index) const {
    // Java reference: NBTTagList.getCompoundTagAt()
    if (index < 0 || index >= static_cast<int32_t>(tags_.size())) return nullptr;
    if (tags_[index]->getId() == TagType::Compound) {
        return static_cast<NBTTagCompound*>(tags_[index].get());
    }
    return nullptr;
}

double NBTTagList::getDoubleAt(int32_t index) const {
    // Java: returns 0.0 if out of bounds or wrong type
    if (index < 0 || index >= static_cast<int32_t>(tags_.size())) return 0.0;
    if (tags_[index]->getId() == TagType::Double) {
        return static_cast<NBTTagDouble*>(tags_[index].get())->getDouble();
    }
    return 0.0;
}

float NBTTagList::getFloatAt(int32_t index) const {
    if (index < 0 || index >= static_cast<int32_t>(tags_.size())) return 0.0f;
    if (tags_[index]->getId() == TagType::Float) {
        return static_cast<NBTTagFloat*>(tags_[index].get())->getFloat();
    }
    return 0.0f;
}

const std::vector<int32_t>& NBTTagList::getIntArrayAt(int32_t index) const {
    static const std::vector<int32_t> empty;
    if (index < 0 || index >= static_cast<int32_t>(tags_.size())) return empty;
    if (tags_[index]->getId() == TagType::IntArray) {
        return static_cast<NBTTagIntArray*>(tags_[index].get())->getIntArray();
    }
    return empty;
}

std::string NBTTagList::getStringTagAt(int32_t index) const {
    // Java: returns "" if out of bounds; returns getString() if String, else toString()
    if (index < 0 || index >= static_cast<int32_t>(tags_.size())) return "";
    if (tags_[index]->getId() == TagType::String) {
        return static_cast<NBTTagString*>(tags_[index].get())->getString();
    }
    return tags_[index]->toString();
}

} // namespace nbt
} // namespace mccpp
