#pragma once

#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/RTTI/BehaviorContext.h>

#include <AzCore/std/tuple.h>
#include <AzCore/std/containers/stack.h>
#include "Utils/Log.h"



namespace spark
{

	class FilterResult {
	public:

		AZ_TYPE_INFO(FilterResult, "{38ACF868-78D8-45BE-915B-5B4AA44315DC}");
		static void Reflect(AZ::ReflectContext* reflection);

		enum FilterAction {
			FILTER_IGNORE,
			FILTER_MODIFY,
			FILTER_PREVENT,
			FILTER_FORCE,
		}action = FILTER_IGNORE;

		void Push();
		void Pop();

		virtual void Clear() 
		{
			action = FILTER_IGNORE;
		}
		virtual void Print()
		{
			AZ_Printf(0, "FilterResult (base)");
		}
		virtual AZStd::string ToString()
		{
			return AZStd::string::format("FilterResult[base]( action:%d )", action);
		}

		static AZStd::stack<FilterResult*>& GetStack();
		static FilterResult* GetStackTop();

		static void SetResult(FilterResult &result) {
			auto &s = GetStack();
			if (!s.empty())
			{
				if (result.action >= s.top()->action)
				{
					*s.top() = result;
				}
			}
		}

	private:
		virtual void ScriptConstructor(AZ::ScriptDataContext& dc)
		{
			AZ_Printf(0, "default ScriptConstructor");
		}

		static void FilterResultScriptConstructor(FilterResult* self, AZ::ScriptDataContext& dc);
	};



	namespace detail
	{
		template<int... Is>
		struct seq { };

		template<int N, int... Is>
		struct gen_seq : gen_seq<N - 1, N - 1, Is...> { };

		template<int... Is>
		struct gen_seq<0, Is...> : seq<Is...> { };

		template<typename T, typename F, int... Is>
		void for_each(T&& t, F &f, seq<Is...>)
		{
			auto l = { (f(std::get<Is>(t)), 0)... };
		}
	}

	template<typename... Ts, typename F>
	void for_each_in_tuple(std::tuple<Ts...>& t, F &f)
	{
		detail::for_each(t, f, detail::gen_seq<sizeof...(Ts)>());
	}

	//c++17 version
	//template<int index = 0, typename Tuple, typename First, typename ...Rest>
	//void copy_tuple_to_args(Tuple &t, First &f, Rest &...rest)
	//{
	//	//if constexpr (index < AZStd::tuple_size<Tuple>::value)   f = AZStd::get< index >(t);
	//	//if constexpr (index < AZStd::tuple_size<Tuple>::value - 1) copy_tuple_to_args<index + 1>(t, rest...);
	//}

	//****//pre c++17//****//
	template<int index = 0, typename Tuple, typename First>
	void copy_tuple_to_args(Tuple &t, First &f)
	{
		if (index < AZStd::tuple_size<Tuple>::value)   f = AZStd::get< index >(t);
	}

	template<int index = 0, typename Tuple, typename First, typename ...Rest>
	void copy_tuple_to_args(Tuple &t, First &f, Rest &...rest)
	{
		if (index < AZStd::tuple_size<Tuple>::value)   f = AZStd::get< index >(t);
		if (index < AZStd::tuple_size<Tuple>::value - 1) copy_tuple_to_args<index + 1>(t, rest...);
	}
	//********//

	template<typename ...Ts>
	class FilterResultTuple : public FilterResult
	{
		struct functor
		{
			AZ::ScriptDataContext& dc;
			int index = 0;

			functor(AZ::ScriptDataContext& dc) :dc(dc) {}

			//pre c++17 version//
			template <typename Generic, class Enable = void>
			void readArg(Generic& t)
			{
				if (!dc.IsClass<Generic>(index)) {
					sERROR("FilterResultTuple: wrong class (index " + index + ")!");
					return;
				}
				dc.ReadArg(index, t);
			}
			template <typename Number, typename std::enable_if_t<std::is_arithmetic<Number>::value>::type >
			void readArg(Number& t)
			{
				if (!dc.IsNumber(index))
				{
					sERROR("FilterResultTuple: wrong class (index " + index + ")! (expected a number value)");
					return;
				}
				dc.ReadArg(index, t);
			}
			void readArg(AZStd::string& t)
			{
				if (!dc.IsString(index))
				{
					sERROR("FilterResultTuple: wrong class (index " + index + ")! (expected a string value)");
					return;
				}
				const char* str = nullptr;
				dc.ReadArg(index, str);
				if (str)t = AZStd::string(str);
			}
			//-----//


			template<typename T>
			void operator () (T& t)
			{
				//we start from index 1, the argument in position 0 is the filter action
				index++;

				if (index >= dc.GetNumArguments()) {
					//sERROR("FilterResultTuple: parameters are missing in lua!");
					return;
				}

				//pre c++17 version//
				readArg(t);

				//c++17 version//
				/*if constexpr (AZStd::is_arithmetic<T>::value)
				{
					if (!dc.IsNumber(index))
					{
						sERROR("FilterResultTuple: wrong class (index "+index+")! (expected a number value)");
						return;
					}
					dc.ReadArg(index, t);
				}
				else if constexpr (AZStd::is_same<T,AZStd::string>::value)
				{
					if (!dc.IsString(index))
					{
						sERROR("FilterResultTuple: wrong class (index " + index + ")! (expected a string value)");
						return;
					}
					const char* str = nullptr;
					dc.ReadArg(index, str);
					if (str)t = AZStd::string(str);
				}
				else
				{
					if (!dc.IsClass<T>(index)) {
						sERROR("FilterResultTuple: wrong class (index " + index + ")!");
						return;
					}
					dc.ReadArg(index, t);
				}*/
			}
		};
		struct ToStringFunctor
		{
			AZStd::string string;
			int index = 0;
			template<typename T>
			void operator () (T& t)
			{
				spark::log::DebugString str = spark::log::DebugString() + "(" + index + "):" + t + ",";
				string += str.data;
				++index;
			}
		};
	public:
		using Tuple = AZStd::tuple<Ts...>;
		Tuple data;

		void ScriptConstructor(AZ::ScriptDataContext& dc)
		{
			for_each_in_tuple(data, functor(dc));
			//sLOG("ScriptConstructor:    " + ToString());
		}
		virtual AZStd::string ToString()
		{
			ToStringFunctor f;
			for_each_in_tuple(data, f);

			AZStd::string str = f.string;
			if (!str.empty())str.pop_back();//remove the last comma

			return AZStd::string::format("FilterResultTuple( action:%d , %s)", action, str.c_str());
		}
		virtual void Print()
		{
			sLOG(ToString());
		}

		template<typename ...Arg>
		void CopyTo(Arg &...arg)
		{
			copy_tuple_to_args(data, arg...);
		}
	};



	template<typename Bus, typename Event, typename ...Args>
	auto SendFilterEvent(Event e, Args ...arg)
	{
		FilterResultTuple<Args...> result;
		result.Push();

		Bus::Broadcast(e, arg...);

		result.Pop();

		return  result;
	}

	template<typename Bus, typename Address, typename Event, typename ...Args>
	auto SendFilterEventId(Address id, Event e, Args ...arg)
	{
		FilterResultTuple<Args...> result;
		result.Push();

		Bus::Event(id, e, arg...);

		result.Pop();

		return  result;
	}

	template<typename Bus, typename Event, typename ...Args>
	bool SendFilterEventPrevent(Event e, Args &...arg)
	{
		FilterResultTuple<Args...> result;
		result.Push();

		Bus::Broadcast(e, arg...);

		result.Pop();

		switch (result.action)
		{
		case FilterResult::FILTER_PREVENT:
			return true;
		case FilterResult::FILTER_MODIFY:
			result.CopyTo(arg...);
		}
		return false;
	}

	template<typename Bus, typename Address, typename Event, typename ...Args>
	bool SendFilterEventIdPrevent(Address id, Event e, Args &...arg)
	{
		FilterResultTuple<Args...> result;
		result.Push();

		Bus::Event(id, e, arg...);

		result.Pop();

		switch (result.action)
		{
		case FilterResult::FILTER_PREVENT:
			return true;
		case FilterResult::FILTER_MODIFY:
			result.CopyTo(arg...);
		}
		return false;
	}

	template<typename Bus, typename Event>
	bool SendFilterEventPrevent(Event e)
	{
		FilterResult result;
		result.Push();

		Bus::Broadcast(e);

		result.Pop();

		switch (result.action)
		{
		case FilterResult::FILTER_PREVENT:
			return true;
		}
		return false;
	}

	template<typename Bus, typename Address, typename Event>
	bool SendFilterEventIdPrevent(Address id, Event e)
	{
		FilterResult result;
		result.Push();

		Bus::Event(id, e);

		result.Pop();

		switch (result.action)
		{
		case FilterResult::FILTER_PREVENT:
			return true;
		}
		return false;
	}


#define SPARK_FILTER_EVENT(BUS, EVENT, ...) SendFilterEvent<BUS>(&BUS::Events::EVENT, __VA_ARGS__)
#define SPARK_FILTER_EVENT_ID(ID,BUS, EVENT, ...) SendFilterEventId<BUS>(ID,&BUS::Events::EVENT, __VA_ARGS__)


#define SPARK_FILTER_EVENT_PREVENT(BUS, EVENT, ...) SendFilterEventPrevent<BUS>(&BUS::Events::EVENT, __VA_ARGS__)
#define SPARK_FILTER_EVENT_ID_PREVENT(ID,BUS, EVENT, ...) SendFilterEventIdPrevent<BUS>(ID,&BUS::Events::EVENT, __VA_ARGS__)
}
