#pragma once
#ifndef LIBOSCAR_KVSTATS_H
#define LIBOSCAR_KVSTATS_H

#include <sserialize/containers/CFLArray.h>
#include <liboscar/OsmKeyValueObjectStore.h>

#include <unordered_map>

namespace liboscar {
namespace detail {
namespace KVStats {
	
struct Data {
	std::unordered_map<std::pair<uint32_t, uint32_t>, uint32_t> keyValueCount;
	Data();
	Data(const Data & other) = default;
	Data(Data && other);
	Data & operator=(Data && other);
	void update(const Static::OsmKeyValueObjectStore::KVItemBase & item);
	static Data merge(Data && first, Data && second);
};

struct ValueInfo {
	uint32_t valueId{ std::numeric_limits<uint32_t>::max() };
	uint32_t count{0};
};

struct KeyInfo {
	KeyInfo();
	uint32_t keyId{ std::numeric_limits<uint32_t>::max() };
	uint32_t count{0};
	sserialize::CFLArray< std::vector<ValueInfo> > values;
};

struct KeyInfoPtr {
	uint32_t offset{ std::numeric_limits<uint32_t>::max() };
	inline bool valid() const { return offset != std::numeric_limits<uint32_t>::max(); }
};

struct State {
	const Static::OsmKeyValueObjectStore & store;
	const sserialize::ItemIndex & items;
	std::atomic<std::size_t> pos{0};
	
	std::mutex lock;
	std::vector<Data> d;
	
	State(const Static::OsmKeyValueObjectStore & store, const sserialize::ItemIndex & items);
};

struct Worker {
	Data d;
	State * state;
	Worker(State * state);
	Worker(const Worker & other);
	void operator()();
	void flush();
};

class Stats {
public:
	using ValueInfo = liboscar::detail::KVStats::ValueInfo;
	using KeyInfo = liboscar::detail::KVStats::KeyInfo;
public:
	Stats(std::unique_ptr<std::vector<ValueInfo>> && valueInfoStore, std::vector<KeyInfo> && keyInfoStore, std::unordered_map<uint32_t, KeyInfoPtr> && keyInfo);
	Stats(Stats && other);
	Stats & operator=(Stats && other);
public:
	KeyInfo & keyInfo(uint32_t keyId);
	const KeyInfo & keyInfo(uint32_t keyId) const;
private:
	std::unique_ptr<std::vector<ValueInfo>> m_valueInfoStore;
	std::vector<KeyInfo> m_keyInfoStore;
	std::unordered_map<uint32_t, KeyInfoPtr> m_keyInfo; //keyId -> keyInfoStore
};

	
}} //end namespace detail::KVStats
	
class KVStats final {
public:
	using ValueInfo = detail::KVStats::ValueInfo;
	using KeyInfo = detail::KVStats::KeyInfo;
	using Stats = detail::KVStats::Stats;
public:
	KVStats(const Static::OsmKeyValueObjectStore & other);
public:
	Stats stats(const sserialize::ItemIndex & items, uint32_t threadCount = 1);
private:
	Static::OsmKeyValueObjectStore m_store;
};

}//end namespace


#endif
