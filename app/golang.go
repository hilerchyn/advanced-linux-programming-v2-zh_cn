package main

import "C"

//import "fmt"
import "os"

//export module_generate
func module_generate(fd int) {
	file := os.NewFile(uintptr(fd), "server")
	_, err := file.Write([]byte(`my data`))
	if err != nil {
		return
	}
	file.Close()
}

func main() {
	// We need the main function to make possible
	// CGO compiler to compile the package as C shared       library
}
