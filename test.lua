function myPrint1(b)
    local a = 1
    local c = myPrint(a+b)
end

function myPrint(b)
	local a = 1
    return b
end

myPrint1(1)