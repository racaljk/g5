package main
type readOnly struct {
	m       map[interface{}]*entry
	amended bool // true if the dirty map contains some key not in m.
}

type arr2d [2*N] struct { x, y int32 }

type (
	A1 = string
	A2 = A1
)
type(
    C
    D
    E
)

type (
	B1 string
	B2 B1
	B3 []B1
	B4 B3)

type arrbyte [32]byte

type arrptr [1000]*float64
type twodimension [3][5]int
type threedimension [2][2][2]float64  // same as [2]([2]([2]float64))