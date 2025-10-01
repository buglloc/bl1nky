package bl1nky

import (
	"fmt"
)

type ErrCode uint8

const (
	ErrorCodeNoDev = iota
	ErrorCodeDevBusy
)

type Error struct {
	Code ErrCode
	Msg  string
}

func NewError(code ErrCode, msg string) *Error {
	return &Error{
		Code: code,
		Msg:  msg,
	}
}

func (e *Error) Is(err error) bool {
	o, ok := err.(*Error)
	if !ok {
		return false
	}

	if e == nil && o == nil {
		return true
	}
	if e == nil || o == nil {
		return false
	}

	return e.Code == o.Code
}

func (e *Error) Error() string {
	if e.Msg == "" {
		return fmt.Sprintf("HwError[%d]: %s", e.Code, errCodeString(e.Code))
	}

	return fmt.Sprintf("HwError[%d]: %s: %s", e.Code, errCodeString(e.Code), e.Msg)
}

func (e *Error) IsPermanent() bool {
	return e.Code == ErrorCodeNoDev
}

func errCodeString(c ErrCode) string {
	switch c {
	case ErrorCodeNoDev:
		return "bl1nky device not found"
	case ErrorCodeDevBusy:
		return "bl1nky device busy"
	default:
		return "unknown"
	}
}
