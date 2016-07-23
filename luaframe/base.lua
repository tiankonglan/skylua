package.cpath = "luaclib/?.so"

local core = require "core"
print("framework  start  --------- ")

-- 调用主函数
local function main()
    print("main runing ----------- ")
    core.callback(dispatch_message)
 end   

 --暴露一个函数给C层调用
function dispatch_message(type)
    print("type ------------ ", type)
    --此处进行分发处理
end

main()





