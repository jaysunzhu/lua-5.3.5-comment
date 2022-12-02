-- exclude = {}
-- function tr(t,prefix)
-- 	if type(t) ~= "table" then return;end
-- 	prefix = prefix.."\t"
-- 	for k,v in pairs(t) do
-- 		print(prefix..tostring(k).."="..tostring(v))
-- 		if exclude[v] then 
-- 		else
-- 			exclude[v]=true
-- 			if type(v) == "table" and k ~= "exclude" then
-- 				tr(v,prefix)
-- 			end
-- 		end
-- 	end
-- end
-- tr(_G,"")
-- print("11111111111111111111")

-- tr(debug.getregistry(),"")

local function localXpcall()
	
	print("localXpcall 1")
	local zero = true
	local strzero = ""..zero
	
	print("localXpcall 2")
end


function main()
	

    local function func()
		print("func 1")
		local yield = coroutine.yield("Hello")
		print("func 2 ".. yield)
		local res, keyString =  pcall(localXpcall)
		print("func 3 ".. tostring(keyString))
    end

	print("main 1")
	local thread = coroutine.create(func)
	print("main 2")
	local res, msg = coroutine.resume(thread,"Lua")
	print("main 3 "..tostring(res)..msg)
	res, msg = coroutine.resume(thread,"World")
	print("main 4 "..tostring(res))
end

main()