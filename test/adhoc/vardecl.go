package main

var smallPowersOfTen = [...]extFloat{
	{1 << 63, -63, false},        // 1
	{0xa << 60, -60, false},      // 1e1
}

var digestSizes = []uint8{
	MD4:         16,
	MD5:         16,
	SHA1:        20,
}
var zeroProcAttr ProcAttr
var zeroSysProcAttr SysProcAttr