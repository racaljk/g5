package main 

type k int

type Regexp struct {
	// read-only after Compile
	regexpRO

	// cache of machines for running regexp
	mu      sync.Mutex
	machine []*machine
}

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

type Point3D struct { x, y, z float64 }
type Line struct { p, q Point3D }