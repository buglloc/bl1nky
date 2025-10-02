package pattern

import "github.com/buglloc/bl1nky"

type ExecutorTracer func(line int, command Command)

type ExecutorOption func(*Executor)

func WithBlinker(blinker bl1nky.Blinker) ExecutorOption {
	return func(e *Executor) {
		e.blinker = blinker
	}
}

func WithTracer(tracer ExecutorTracer) ExecutorOption {
	return func(e *Executor) {
		e.tracer = tracer
	}
}
