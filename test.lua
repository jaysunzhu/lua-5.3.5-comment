
local b = 10
function myPrint1(a)
    if a == 0 then
        return
    end
    -- print("myPrint1:"..a)
    a = a - 1
    return myPrint2(a)
end
function myPrint2(a)
    if a == 0 then
        return
    end
    -- print("myPrint2:"..a)
    a = a - 1
    return myPrint1(a)
end
myPrint1(100)