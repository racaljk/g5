package main 
type Point3D struct { x, y, z float64}
type Line struct { p, q Point3D }
type k int

type Regexp struct {
	// read-only after Compile
	regexpRO

	// cache of machines for running regexp
	mu      sync.Mutex
	machine []*machine}

type regexpRO struct {
	expr           string         // as passed to Compile
	prog           *syntax.Prog   // compiled program
	onepass        *onePassProg   // onepass program or nil
	prefix         string         // required prefix in unanchored matches
	prefixBytes    []byte         // prefix, as a []byte
	prefixComplete bool           // prefix is the entire regexp
	prefixRune     rune           // first rune in prefix
	prefixEnd      uint32         // pc for last rune in prefix
	cond           syntax.EmptyOp // empty-width conditions required at start of match
	numSubexp      int
	subexpNames    []string
	longest        bool
}
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

