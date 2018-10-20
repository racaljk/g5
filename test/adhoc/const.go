package main

const endOfText rune = -1
// The result of Scan is one of these tokens or a Unicode character.
const (
	EOF = -(iota + 1)
	Ident
	Int
	Float
	Char
	String
	RawString
	Comment
	skipComment)

const k = 5

const (
	ScanIdents     = 1 << -Ident
	ScanInts       = 1 << -Int
	ScanFloats     = 1 << -Float // includes Ints
	ScanChars      = 1 << -Char
	ScanStrings    = 1 << -String
	ScanRawStrings = 1 << -RawString
	ScanComments   = 1 << -Comment
	SkipComments   = 1 << -skipComment // if set with ScanComments, comments become white space
	GoTokens       = ScanIdents | ScanFloats | ScanChars | ScanStrings | ScanRawStrings | ScanComments | SkipComments
)


const(
    kk
	aa,bb,cc int=7,2,4
	dd,ee,ff
)
const (
	Txxx = types.Txxx

	TINT8    = types.TINT8
	TUINT8   = types.TUINT8
	TINT16   = types.TINT16
	TUINT16  = types.TUINT16
	TINT32   = types.TINT32
	TUINT32  = types.TUINT32
	TINT64   = types.TINT64
	TUINT64  = types.TUINT64
	TINT     = types.TINT
	TUINT    = types.TUINT
	TUINTPTR = types.TUINTPTR

	TCOMPLEX64  = types.TCOMPLEX64
	TCOMPLEX128 = types.TCOMPLEX128

	TFLOAT32 = types.TFLOAT32
	TFLOAT64 = types.TFLOAT64

	TBOOL = types.TBOOL

	TPTR32 = types.TPTR32
	TPTR64 = types.TPTR64

	TFUNC      = types.TFUNC
	TSLICE     = types.TSLICE
	TARRAY     = types.TARRAY
	TSTRUCT    = types.TSTRUCT
	TCHAN      = types.TCHAN
	TMAP       = types.TMAP
	TINTER     = types.TINTER
	TFORW      = types.TFORW
	TANY       = types.TANY
	TSTRING    = types.TSTRING
	TUNSAFEPTR = types.TUNSAFEPTR

	// pseudo-types for literals
	TIDEAL = types.TIDEAL
	TNIL   = types.TNIL
	TBLANK = types.TBLANK

	// pseudo-types for frame layout
	TFUNCARGS = types.TFUNCARGS
	TCHANARGS = types.TCHANARGS

	// pseudo-types for import/export
	TDDDFIELD = types.TDDDFIELD // wrapper: contained type is a ... field

	NTYPE = types.NTYPE
)