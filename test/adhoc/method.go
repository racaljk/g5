package main

type SymID struct {
	Name string
	Version int64
}

func (s SymID) String() string {
}
func (s *SymID) String() string {
}