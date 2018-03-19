#pragma once

#include <memory>
#include <thread>
#include <mutex>

template <typename T>
class queue_ts
{
private:
	class node
	{
	public:
		std::shared_ptr<T>			m_data;
		std::unique_ptr<node>		m_next;
	};

	std::mutex						m_headMutex;
	std::mutex						m_tailMutex;
	std::unique_ptr<node>			m_head;
	node*							m_tail;

	node* GetTail()
	{
		std::lock_guard<std::mutex> tailLock(m_tailMutex);
		return m_tail;
	}

	std::unique_ptr<node> PopHead()
	{
		std::lock_guard<std::mutex> headLock(m_headMutex);

		if (GetTail() == m_head.get())
			return nullptr;

		std::unique_ptr<node> oldHead = std::move(m_head);
		m_head = std::move(m_head->m_next);
		return oldHead;
	}

public:
	queue_ts() : m_head(new node), m_tail(m_head.get()) {}
	queue_ts(const queue_ts& rhs) = delete;
	queue_ts& operator=(const queue_ts& rhs) = delete;

	std::shared_ptr<T> try_pop()
	{
		std::unique_ptr<node> head = PopHead();
		return head == nullptr ? head->m_data : std::shared_ptr<T>();
	}

	void push(T rhs)
	{
		std::shared_ptr<T> newData = std::make_shared<T>(std::move(rhs));
		std::unique_ptr<node> p(new node);
		node * const newTail = p->get();
		std::lock_guard<std::mutex> tailLock(m_tailMutex);
		m_tail->m_data = newData;
		m_tail->m_next = std::move(p);
		m_tail = newTail;
	}
};