local a = 1
local b = 1
function myPrint1(c)

    local c = myPrint(a+c)
end

function myPrint(c)
	b = 10
    return c
end

myPrint1(1)