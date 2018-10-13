package main

import (
	"fmt"
	_ "go/ast"
	"errors"
	"strconv"
	"strings"
)

func div(a,b float64) (float64, error){
	if b==0{
		return 0,errors.New("should not divide by zero")
	}
	return a/b,nil;
}

func info(num int8){
	switch num {
	case 1:
		fmt.Println("into 1st")
	case 2:
		fmt.Println("into 2nd")
	case 3:
		fmt.Println("into 3rd")
	}
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
	for i:=0;i<len(slice);i++{
		fmt.Println(slice[i])
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
	for i:=0;i<5;i++{
		fmt.Println("goroutine:",startID,"- number:",i)
	}
}