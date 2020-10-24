#pragma once
#include <vector>
#include "IComponent.h"

namespace VECTOR_HELPERS
{
	template <class T> static void RemoveItemFromVector(std::vector<T*>& items, T*& item)
	{
		for (int i = 0; i < items.size(); i++)
		{
			if (items[i] == item)
			{
				// switch place on the component last in list with the component we want to remove
				T* temp = items.back();
				items.back() = items[i];
				items[i] = temp;

				// remove the last component
				items.pop_back();
				return;
			}
		}
	}
}