package skynet

import "sync"

type NonBlockingQueue struct {
	data  []interface{}
	mutex sync.Mutex
}

func NewNonBlockingQueue() *NonBlockingQueue {
	return &NonBlockingQueue{
		data: make([]interface{}, 0),
	}
}

func (q *NonBlockingQueue) Enqueue(item interface{}) {
	q.mutex.Lock()
	q.data = append(q.data, item)
	q.mutex.Unlock()
}

func (q *NonBlockingQueue) Dequeue() (interface{}, bool) {
	q.mutex.Lock()
	defer q.mutex.Unlock()

	if len(q.data) == 0 {
		return nil, false
	}

	item := q.data[0]
	q.data = q.data[1:]
	return item, true
}

func (q *NonBlockingQueue) Len() int {
	q.mutex.Lock()
	defer q.mutex.Unlock()
	return len(q.data)
}
