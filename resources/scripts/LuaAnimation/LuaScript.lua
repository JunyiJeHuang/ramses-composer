modules('CurveModule', 'DataModule', 'SharedModule', 'UtilModule', 'LuaJsonDataModule')


DataModule.init('LuaJsonDataModule', LuaJsonDataModule)
DataModule.init('UtilModule', UtilModule)
local curve, outputNameMapping, defaultValues = DataModule.getTransformCurveData()
SharedModule.init('UtilModule', UtilModule)
SharedModule.init('defaultValues', defaultValues)

-- 调试使用
local DEBUG = false

function interface(IN, OUT)
    IN.ticker = Type:Int64()
    IN.currentFrame = Type:Int64()
    OUT.AAcurrentFrame = Type:Int64()
    IN.start = Type:Bool()
    -- OUT.ticker = Type:Int64()
    -- OUT.poly_surface_translation = Type:Vec3f()
    -- OUT.poly_surface_scaling = Type:Vec3f()
    -- OUT.poly_surface_move_range = Type:Float()
    -- OUT.ellie_body_translation = Type:Vec3f()
    -- OUT.ellie_body_scaling = Type:Vec3f()
    -- OUT.current_frame = Type:Int64()

    for k in pairs(curve) do
        local mapping = outputNameMapping[k]
        local outputName = nil
        if DEBUG then
            outputName = k
        else
            outputName = mapping.objectId .. '.' .. mapping.property
        end         
        if mapping.type == 'double' then
            OUT[outputName] = Type:Float()
        elseif mapping.type == 'vec3f' then
            OUT[outputName] = Type:Vec3f()
        end
    end
end

function init()
    local animationConfig = DataModule.getActiveAnimationConfig()
    --  updateInterval
    -- 动画总帧数
    GLOBAL.frames = animationConfig['end'] or 100
    -- 每秒帧数
    GLOBAL.frame = 1000 / (animationConfig.updateInterval or 24)
    -- 一秒内时间开始标记
    GLOBAL.startTick = 0
    -- 当前帧
    GLOBAL.currentFrame = 1
end

function run(IN, OUT)
    if IN.start == true then
        local microseconds = IN.ticker / 1000
        -- 首次运行设置开始时间标记
        if GLOBAL.startTick == 0 then
            GLOBAL.startTick = microseconds
        end

        local dis = microseconds - GLOBAL.startTick

        local progress = math.floor(dis / (1000 / GLOBAL.frame)) + 1
        GLOBAL.currentFrame = progress
    else
        GLOBAL.startTick = 0
        GLOBAL.currentFrame = 1
    end

    if GLOBAL.currentFrame <= GLOBAL.frames then
        OUT.AAcurrentFrame = GLOBAL.currentFrame
    else
        GLOBAL.currentFrame = GLOBAL.frames
    end

    -- GLOBAL.currentFrame = IN.currentFrame
    local res = SharedModule.getCurrentPoints(curve, GLOBAL.currentFrame)

    for on, ci in pairs(res) do
        -- local veclen = #ci
        local o = {
            x = 0,
            y = 0,
            z = 0
        }
        local valueType = ci.type

        for k, pi in pairs(ci.value) do
            if pi.next ~= nil and pi.prev ~= nil then
                -- 存在曲线计算的值
                if pi.prev.InterpolationType == 0 then
                    -- Liner
                    o[k] = CurveModule.calculateLiner(GLOBAL.currentFrame, pi.prev.keyFrame, pi.prev.Data,
                    pi.next.keyFrame, pi.next.Data)
                elseif pi.prev.InterpolationType == 1 then
                    -- Hermite
                    o[k] = CurveModule.calculateHermite(GLOBAL.currentFrame, pi.prev.keyFrame, pi.prev.Data,
                    pi.prev.RightData, pi.next.keyFrame, pi.next.Data, pi.next.LeftData)
                end
            else
                -- 需要设置默认值
                o[k] = pi.default
            end
        end
        local mapping = outputNameMapping[on]

        local outputName = nil
        if DEBUG then
            outputName = on
        else
            outputName = mapping.objectId .. '.' .. mapping.property
        end

        if valueType == 'double' then
            OUT[outputName] = o.x
        elseif valueType == 'vec3f' then
            OUT[outputName] = { o.x, o.y, o.z }
        end
    end
end
