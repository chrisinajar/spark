#pragma once


#include <AzCore/std/containers/vector.h>


namespace spark {

	template<typename T>
	class CircularVector : public AZStd::vector<T>
	{
		int index = 0;
		T invalid;
	public:
		T& GetCurrent() {	
			return this->empty() ? invalid : this->at(GetIndex());
		}
		T GetCurrent() const{
			return this->empty() ? invalid : this->at(GetIndex());
		}

		T& GetNext() {
			SetIndex(GetIndex() + 1);
			return GetCurrent();
		}
		T& GetPrevious() {
			SetIndex(GetIndex() - 1);
			return GetCurrent();
		}


		int GetIndex() const {
			return index;
		}
		void SetIndex(int i) { 
			index = this->empty() ? 0 : i % this->size();
			index = index < 0 ? this->size() + index : index;
		}
		void clear() {
			AZStd::vector<T>::clear();
			index = 0;
		}
	};

}