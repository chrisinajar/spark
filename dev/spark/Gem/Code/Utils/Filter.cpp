#include "spark_precompiled.h"

#include "Filter.h"


#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>


namespace spark
{
	static AZStd::stack<FilterResult*> resultsStack;

	void FilterResult::FilterResultScriptConstructor(FilterResult* self, AZ::ScriptDataContext& dc)
	{
		
		if (dc.GetNumArguments() == 0)
		{
			*self = FilterResult();
			return;
		}
		else if (dc.GetNumArguments() >= 1)
		{
			new(self) FilterResult{ };

			if (dc.IsNumber(0))
			{
				int type = 0;
				dc.ReadArg(0, type);	
				self->action = (FilterResult::FilterAction)type;

				if (!resultsStack.empty()) {
					//when there are multiple handler of the same filter, do the thing only if the priority is greater or equal
					auto &top = resultsStack.top();
					if (self->action >= top->action)
					{
						top->action = self->action;
						top->ScriptConstructor(dc);
					}
				}
				return;
			}
			
		}

		dc.GetScriptContext()->Error(AZ::ScriptContext::ErrorType::Error, true, "Invalid arguments passed to FilterResult().");
		new(self) FilterResult();
	}


	void FilterResult::Reflect(AZ::ReflectContext* reflection)
	{
		if (auto serializationContext = azrtti_cast<AZ::SerializeContext*>(reflection))
		{
			serializationContext->Class<FilterResult>()
				->Version(1)
				->Field("action", &FilterResult::action);
		}

		if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(reflection))
		{
			behaviorContext->Class<FilterResult>("FilterResult")
				->Attribute(AZ::Script::Attributes::Storage, AZ::Script::Attributes::StorageType::Value)
				->Attribute(AZ::Script::Attributes::ConstructorOverride, &FilterResultScriptConstructor)
				
				->Enum<(int)FilterResult::FILTER_IGNORE>("FILTER_IGNORE")
				->Enum<(int)FilterResult::FILTER_PREVENT>("FILTER_PREVENT")
				->Enum<(int)FilterResult::FILTER_MODIFY>("FILTER_MODIFY")
				->Enum<(int)FilterResult::FILTER_FORCE>("FILTER_FORCE")
				;
		}
	}


	void FilterResult::Push()
	{
		resultsStack.push(this);
	}
	void FilterResult::Pop()
	{
		AZ_Assert(!resultsStack.empty(), "FilterResult::Pop() called, but stack is empty!");
		AZ_Assert(resultsStack.top()==this, "FilterResult::Pop() called, but the top of the stack does not match!");
		
		resultsStack.pop();
	}

	AZStd::stack<FilterResult*>& FilterResult::GetStack()
	{
		return resultsStack;
	}

	FilterResult* FilterResult::GetStackTop()
	{
		return resultsStack.empty() ? nullptr : resultsStack.top();
	}
}
