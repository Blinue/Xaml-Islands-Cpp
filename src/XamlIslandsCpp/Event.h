#pragma once
#include <functional>

namespace XamlIslandsCpp {

struct EventToken {
	uint32_t value = 0;

	explicit operator bool() const noexcept {
		return value != 0;
	}
};

#ifdef _DEBUG
inline int _DEBUG_DELEGATE_COUNT = 0;
#endif

// 简易且高效的事件，不支持在回调中修改事件本身
template <typename... TArgs>
class Event {
private:
	using _FunctionType = std::function<void(TArgs...)>;

public:
#ifdef _DEBUG
	~Event() {
		_DEBUG_DELEGATE_COUNT -= (int)_delegates.size();
	}
#endif

	template <typename T>
	EventToken operator()(T&& handler) {
#ifdef _DEBUG
		++_DEBUG_DELEGATE_COUNT;
#endif
		_delegates.emplace_back(++_curToken, std::forward<T>(handler));
		return { _curToken };
	}

	void operator()(EventToken token) noexcept {
#ifdef _DEBUG
		--_DEBUG_DELEGATE_COUNT;
#endif
		auto it = std::find_if(_delegates.begin(), _delegates.end(),
			[token](const std::pair<uint32_t, _FunctionType>& d) { return d.first == token.value; });
		assert(it != _delegates.end());
		_delegates.erase(it);
	}

	class EventRevoker {
	public:
		EventRevoker() noexcept = default;

		EventRevoker(const EventRevoker&) noexcept = delete;
		EventRevoker(EventRevoker&& other) noexcept {
			_Swap(other);
		}

		EventRevoker& operator=(const EventRevoker&) noexcept = delete;
		EventRevoker& operator=(EventRevoker&& other) noexcept {
			EventRevoker(std::move(other))._Swap(*this);
			return *this;
		}

		EventRevoker(Event* event, EventToken token) noexcept : _event(event), _token(token) {}

		~EventRevoker() {
			if (_event) {
				_event->operator()(_token);
			}
		}

		void Revoke() {
			if (_event) {
				_event->operator()(_token);
				_event = nullptr;
			}
		}

	private:
		void _Swap(EventRevoker& other) noexcept {
			std::swap(_event, other._event);
			std::swap(_token, other._token);
		}

		Event* _event = nullptr;
		EventToken _token;
	};

	// 调用者应确保 EventRevoker 在 Event 的生命周期内执行撤销
	template <typename T>
	EventRevoker operator()(winrt::auto_revoke_t, T&& handler) {
		return EventRevoker(this, operator()(std::forward<T>(handler)));
	}

	template <typename... TArgs1>
	void Invoke(TArgs1&&... args) {
		for (auto& pair : _delegates) {
			pair.second(std::forward<TArgs1>(args)...);
		}
	}

private:
	// 考虑到回调数量都很少，std::vector 的性能更好
	std::vector<std::pair<uint32_t, _FunctionType>> _delegates;
	uint32_t _curToken = 0;
};

}
