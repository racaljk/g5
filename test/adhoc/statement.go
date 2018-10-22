package main

import (
	"fmt"
	_ "go/ast"
	"errors"
	"strconv"
	"strings"
)
func newBucket(typ bucketType, nstk int) *bucket {
	size := unsafe.Sizeof(bucket{}) + uintptr(nstk)*unsafe.Sizeof(uintptr(0))
	switch typ {
	default:
		throw("invalid profile bucket type")
	case memProfile:
		size += unsafe.Sizeof(memRecord{})
	case blockProfile, mutexProfile:
		size += unsafe.Sizeof(blockRecord{})
	}

	b := (*bucket)(persistentalloc(size, 0, &memstats.buckhash_sys))
	bucketmem += size
	b.typ = typ
	b.nstk = uintptr(nstk)
	return b
}
func TestChanSendInterface(t *testing.T) {
    size := unsafe.Sizeof(bucket{}) + uintptr(nstk)*unsafe.Sizeof(uintptr(0))
	select {
	case c <- &mt{}:
	default:
	}
    select {
	case c <- m:
	default:
	}
}
func (d *digest) Write(p []byte) (nn int, err error) {
	nn = len(p)
	d.len += uint64(nn)
	if d.nx > 0 {
		n := copy(d.x[d.nx:], p)
		d.nx += n
		if d.nx == chunk {
			block(d, d.x[:])
			d.nx = 0
		}
		p = p[n:]
	}
	if len(p) >= chunk {
		n := len(p) &^ (chunk - 1)
		block(d, p[:n])
		p = p[n:]
	}
	if len(p) > 0 {
		d.nx = copy(d.x[:], p)
	}
	return
}

func selectstmt(){
    select {}  // block forever
    var a []int
    var c, c1, c2, c3, c4 chan int
    var i1, i2 int
    select {
    case i1 = <-c1:
	    print("received ", i1, " from c1\n")
    case c2 <- i2:
	    print("sent ", i2, " to c2\n")
    case i3, ok := (<-c3):  // same as: i3, ok := <-c3
	    if ok {
		    print("received ", i3, " from c3\n")
	    } else {
		    print("c3 is closed\n")
	    }
    case a[f()] = <-c4:
	    // same as:
	    // case t := <-c4
	    //	a[f()] = t
    default:
	    print("no communication\n")
    }

    for {  // send random sequence of bits to c
	    select {
	    case c <- 0:  // note: no statement, no fallthrough, no folding of cases
	    case c <- 1:
	    }
    }
}

func forstmt(){
	for ; i < len(s); i++ {
		if special(s[i]) {
			b[j] = '\\'
			j++
		}
		b[j] = s[i]
		j++
	}

    for i, _ := range testdata.a {
	    // testdata.a is never evaluated; len(testdata.a) is constant
	    // i ranges from 0 to 6
	    f(i)
    }
    for{
        fmt.Println("ffff")
    }

    for a < b {
	    a *= 2
    }

    for i := 0; i < 10; i++ {
	    f(i)
    }
    for key, val = range m {
	    h(key, val)
    }
    // key == last map key encountered in iteration
    // val == map[key]

    var ch chan Work = producer()
    for w := range ch {
	    doWork(w)
    }

    // empty a channel
    for range ch {}
}


func labeledstmt(){
    Error: log.Panic("error encountered")
    OK:
}

func simplestmt(){
    field1, offset := nextField(str, 0)
    field2, offset := nextField(str, offset)  // redeclares offset
    //short assign stmt
    i, j := 0, 10
    ch := make(chan int)
    r, w := os.Pipe(fd)  // os.Pipe() returns two values
    _, y, _ := coord(p)  
    a, a := 1, 2   
    f := func() int { return 7 }
    a[i] <<= 2
    (k) = <-ch  
    a[i] = 23
    i &^= 1<<n
    //expr stmt
    h(x+y)
    f.Close()
    <-ch
    (<-ch)
    // send stmt
    ch <- 3 
    //incdec stmt
    p++
    k--
    //assign stmt
    x = 1
    *p = f()
    x, y = f()                    
}
func switchstmt(p interface{}) {
	switch tag {
        default: s3()
        case 0, 1, 2, 3: s1()
        case 4, 5, 6, 7: s2()
    }

    switch q :=p.(type) {
	case int:
		fmt.Print("int")
	case float64:
		fmt.Print(q)
		fmt.Printf("float64")
	}


    switch x := f(); {  // missing switch expression means "true"
    case x < 0: return -x
    default: return x
    }

    switch {
    case x < y: f1()
    case x < z: f2()
    case x == 4: f3()
    }
    switch i := x.(type) {
    case nil:
	    printString("x is nil")                // type of i is type of x (interface{})
    case int:
	    printInt(i)                            // type of i is int
    case float64:
	    printFloat64(i)                        // type of i is float64
    case func(int) float64:
	    printFunction(i)                       // type of i is func(int) float64
    case bool, string:
	    printString("type is bool or string")  // type of i is type of x (interface{})
    default:
	    printString("don't know the type")     // type of i is type of x (interface{})
    }

    switch num {
	case 1:
		fmt.Println("into 1st")
	case 2:
		fmt.Println("into 2nd")
	case 3:
		fmt.Println("into 3rd")
	}
}

func ifstmt2(a,b float64) (float64, error){
	if b==0{
		return 0,errors.New("should not divide by zero")
	}
	return a/b,nil;
}

func ifstmt(){
    if x > max {
	    x = max
    }
    //The expression may be preceded by a simple statement, which executes before the expression is evaluated.

    if x := f(); x < y {
	    return x
    } else if x > z {
	    return z
    } else {
	    return y
    }
}

func typeAssertion(p interface{}){
	q:=p.(int)
	fmt.Print(q)
}


func closure(x int16) (func()func()int16){
	defer info(1)
	return func() func()int16{
		fmt.Print("in fact, in the statement defer foo(), foo() would be called BEFORE actual returning")
		defer info(2)
		 y:=x+1
		 return func() int16{
		 	defer info(3)
			return y+1
		}
	}
}
func main(){
	var  x int32
	var s string = "hello golang!"
	fmt.Println(x,s)

	if x>0{
		fmt.Print("test")
	}else if x==0{
		fmt.Println("golang")
	}
	switch {
	case x==0:
		fmt.Println("would be")
	case x>0:
		fmt.Println(" fallthrough")
	}

	res,err := div(6,7)
	fmt.Println(res,err)
	res1,err1:=div(6.25,3563.75)
	fmt.Println(res1,err1)
	res2,err2 := div(64,0)
	fmt.Println(res2,err2)
	fmt.Println(nil)
	fmt.Println("deep clousure function:",closure(7)()())

	slice := make([]int,18,20)
	//fmt.Println(slice)
	for p:=0;p<len(slice);p++{
		fmt.Println(slice[p])
	}

	sliceInfo(slice)
	slice = append(slice,3)
	sliceInfo(slice)
	slice = append(slice,5)
	slice = append(slice,87)
	sliceInfo(slice)
	addEmployee(1,"yangyi")
	addEmployee(4,"Jame Smith")
	addEmployee(5,"Scott Meyesh")
	addEmployee(7,"John Kow")
	fireEmployee(5)
	fireEmployee(3)
	fmt.Println(employee)
	var building Dormitory
	building.ownerName ="chthulhu"
	building.roomNum = 65
	building.plate =26.309
	building.userType = "postgraduate"
	fmt.Println(building)
	building.updateAnonymousField()
	fmt.Println(building)
	building.updateAnonymousFieldWithoutPointer()
	fmt.Println(building)
	building.print()

	var printableObject Printable = building
	printableObject.print()
	var _ Anything = building
	var _ Anything = 6.8

	go task(5)
	go task(6)
	// In golang, the program exited as long as the main goroutine accomplished,
	//  it would ignore other goroutines even if they are running.
	//time.Sleep(time.Second*2)

}

var employee map[int]string = make(map[int]string)

func sliceInfo(t []int){
	fmt.Println("the length of slice is "+ strconv.Itoa(len(t))+" and the capacity of the slice is "+ strconv.Itoa(cap(t)))
}

func addEmployee(id int, name string){
	employee[id] = name
}

func fireEmployee(id int){
	delete(employee,id)
}

type House struct{
	roomNum int
	ownerName string
}

type Dormitory struct{
	House 				// anonymous field embedding
	userType string 	// student teacher leader
	plate float32
}

func (obj *Dormitory) updateAnonymousField(){
	obj.ownerName = strings.ToUpper(obj.ownerName)
}

func (obj Dormitory) updateAnonymousFieldWithoutPointer(){
	obj.ownerName = strings.ToLower(obj.ownerName)
}

type Printable interface{
	print()
}

type Anything interface{}

func (obj Dormitory) print(){
	fmt.Println("this function can internally modify state of caller")
}
/**
note that it's illegal to implement an interface if we use pointer of object in the function declaration rather than pure object
func (obj *Dormitory) print(){...}		//bad, type of Dormitory actually do not implement the interface Printable
 */

func task(startID int){
	for xs:=0;i<5;i++{
		fmt.Println("goroutine:",startID,"- number:",i)
	}
}