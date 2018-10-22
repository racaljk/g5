package main 

func initAtofOnce() {
	if t.a != nil {
		f.b = &NumError{"ParseFloat", test.in, test.err}
	}
}

func f(a, b int, z float64, opt ...interface{}) (success bool)

func a()(a,b,c float32,d... float32,...e)
func g(int, int, float64) (float64, *[]int)
func b(x int) int
func c(a, _ int, z float32) bool
func d(a, b int, z float32) (bool)
func e(prefix string, values ...int)

func h(n int) func(p *T)

type (
	B4 func(int, float64) *B0
	B5 func(x int, y float64) *A1
    	B2 struct{ a, b int }
	B3 struct{ a, c int }
)

type	C0 = B0
    type (
	A0 = []string
	A1 = A0
	A2 = struct{ a, b int }
	A3 = int
	A4 = func(A3, float64) *A0
	A5 = func(x int, _ float64) *[]string
)

// A simple File interface
type P interface {
    Dummy(a Buffer, b Buffer, f1,f2,f3 Buffer)
	Read(b Buffer) bool
	Write(b Buffer) bool
	Close()
}
type ReadWriter interface {
	Read(b Buffer) bool
	Write(b Buffer) bool
}

type File interface {
	ReadWriter  // same as adding the methods of ReadWriter
	Locker      // same as adding the methods of Locker
	Close()
}

type LockedFile interface {
	Locker
	File        // illegal: Lock, Unlock not unique
	Lock()      }

        type SymID struct {
	    Name string
	    Version int64
}

func (s SymID) String() string {
}
func (s *SymID) String() string {
}