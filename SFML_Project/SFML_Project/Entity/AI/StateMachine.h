#pragma once
#include <string>
#include <vector>
#include <map>

template <typename T>
class StateMachine
{
public:
	struct State
	{
		std::string Name;
		std::string(T::*NexStateCondition)(double dt);
		void(T::*BehaviourLogic)(double dt);
	};

public:
	StateMachine() = default;
	~StateMachine() = default;

	void SetStates(const std::vector<State> & states);
	void Update(double dt, T * object);
	const std::string & GetCurrentState() const;


private:
	std::map<std::string, State> m_states;
	std::string m_currentState = "";

};

template<typename T>
inline void StateMachine<T>::SetStates(const std::vector<State>& states)
{
	if (!states.empty())
	{
		m_currentState = states.front().Name;
		size_t stateSize = states.size();
		for (size_t i = 0; i < stateSize; i++)
		{
			m_states.insert(std::make_pair(states[i].Name, states[i]));
		}
	}
}

template<typename T>
inline void StateMachine<T>::Update(double dt, T * object)
{
	m_currentState = (object->*(m_states[m_currentState].NexStateCondition))(dt);
	(object->*(m_states[m_currentState].BehaviourLogic))(dt);
}

template<typename T>
inline const std::string & StateMachine<T>::GetCurrentState() const
{
	return m_currentState;
}
