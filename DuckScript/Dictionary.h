#pragma once
#include <map>
#include <vector>
#include <algorithm>

template <typename Key, typename Value>
class Dictionary
{
	public:
		using valueType = std::pair<Key, Value>;
		using containerType = std::vector<valueType>;
		
	private:
		containerType container;
		
	public:
		using iterator = typename containerType::const_iterator;
		using const_iterator = iterator;

		Dictionary(std::initializer_list<std::pair<Key, Value>> init) :
			container(init)
		{
			std::sort(container.begin(), container.end());
		}

		Dictionary(containerType init) :
			container(init)
		{
			std::sort(container.begin(), container.end());
		}

		template <typename K>
		const_iterator find(const K& key) const
		{
			const_iterator it = std::lower_bound(
				container.begin(), 
				container.end(), 
				key,
				[](const valueType& v, const K& key) {
					return v.first < key; 
				}
			);
			return it != container.end() && it->first == key ? it : container.end();
		}

		const_iterator begin() const
		{
			return container.begin();
		}

		const_iterator end() const
		{
			return container.end();
		}

		size_t size() const
		{
			return container.size();
		}
};