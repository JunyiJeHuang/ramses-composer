modules('CurveModule', 'DataModule', 'SharedModule', 'UtilModule', 'LuaJsonDataModule')


DataModule.init('LuaJsonDataModule', LuaJsonDataModule)
DataModule.init('UtilModule', UtilModule)
local interfaceDefinedMap, defaultValues = DataModule.getInterfaceDefinedMap()

SharedModule.init('UtilModule', UtilModule)
SharedModule.init('defaultValues', defaultValues)

local DEBUG = false

-- Animation name list
local animationList = DataModule.getAnimationNames()

function interface(IN, OUT)
    IN.ticker = Type:Int64()

    OUT['0currentFrame'] = Type:Int64()
    -- Current animation playback progress
    OUT.progress = Type:Float()
    -- Start playing
    IN.start = Type:Bool()
    -- When the value is `true`, the animation playback can be controlled separately,
    -- while `false` continuously plays
    IN.playAlone = Type:Bool()
    -- External interface, setting animation names
    IN.animationName = Type:String()
    -- Current animation name
    OUT.animationName = Type:String()
    --[[
        External interface:
        Scale: Scale in pixel size, the value range is 0-1000, and the default value is 300.
        Opacity: For visibility, Hidden when the value is 0, visible when the value is 1, with a default value of 1.
        Right: Orientation of Ellie, on the right when the value is 1, on the left when the value is 0, with a default value of 0.
        Color:  1. Add external variables: HUD, C1, HUD_ C1；
                2. colorMode_ Color1=mix (C1_V4, HUD_C1_V4, HUD) color mixing;
                3. Bind colorMode_ Color1 to u_ color1
        DotOpacity: opacity for bg, value range 0-1, default value 0.75.
	    DotSize: contros the size of the bg, value range 0-1, default value 1.
	    Slide: Switch from StartOffset to TargetOffsest, value range 0-1, default value 0.
    ]]
    IN.Scale = Type:Float()
    OUT.Scale = Type:Vec3f()
    -- OPacity
    IN.Opacity = Type:Float()
    OUT.Opacity = Type:Float()
    -- HUD
    IN.HUD = Type:Float()
    -- u_color1
    IN.Content_IPAIconTransformation_C1 = Type:Vec4f()
    IN.HUD_Content_IPAIconTransformation_C1 = Type:Vec4f()
    OUT.colorMode_Color1 = Type:Vec4f()
    -- u_color2
    IN.Content_IPAIconTransformation_C2 = Type:Vec4f()
    IN.HUD_Content_IPAIconTransformation_C2 = Type:Vec4f()
    OUT.colorMode_Color2 = Type:Vec4f()
    -- u_color3
    IN.Content_IPAIconTransformation_C3 = Type:Vec4f()
    IN.HUD_Content_IPAIconTransformation_C3 = Type:Vec4f()
    OUT.colorMode_Color3 = Type:Vec4f()
    -- DotOpacity
    IN.DotOpacity = Type:Float()
    OUT.dot_opacity = Type:Float()
    -- DotSize
    IN.DotSize = Type:Float()
    OUT.dot_size = Type:Float()
    -- Right
    IN.Right = Type:Int32()
    -- Slide
    IN.Slide = Type:Float()
    IN.StartOffset = Type:Vec2f()
    IN.TargetOffset = Type:Vec2f()
    OUT.slide = Type:Float()
    OUT.start_offset = Type:Vec2f()
    OUT.target_offset = Type:Vec2f()
    OUT.slide_translation = Type:Vec3f()
    -- External interface

    for on, def in pairs(interfaceDefinedMap) do
        local outputName = nil
        if DEBUG then
            outputName = on
        else
            outputName = def.objectId .. '.' .. def.property
        end
        if def.type == 'double' then
            OUT[outputName] = Type:Float()
        elseif def.type == 'vec3f' then
            OUT[outputName] = Type:Vec3f()
        elseif def.type == 'vec2f' then
            OUT[outputName] = Type:Vec2f()
        elseif def.type == 'vec4f' then
            OUT[outputName] = Type:Vec4f()
        end
    end
end

function init()
    local animationConfig = DataModule.getActiveAnimationConfig()
    local activeAnimationName = DataModule.getActiveAnimationName()
    GLOBAL.lastAnimationName = activeAnimationName
    GLOBAL.animationName = activeAnimationName
    -- animation total frames
    GLOBAL.frames = animationConfig['end'] or 100
    -- frame / second
    GLOBAL.frame = 1000 / (animationConfig.updateInterval or 24)

    GLOBAL.startTick = 0

    GLOBAL.currentFrame = 0

    GLOBAL.loopCount = 1

    GLOBAL.animationIdx = 1
end

function run(IN, OUT)
    if IN.start == true then
        local microseconds = IN.ticker / 1000
        -- First run setting start time stamp
        if GLOBAL.startTick == 0 then
            GLOBAL.startTick = microseconds
        end

        local dis = microseconds - GLOBAL.startTick

        local progress = math.floor(dis / (1000 / GLOBAL.frame)) + 1
        GLOBAL.currentFrame = progress
    else
        GLOBAL.startTick = 0
        GLOBAL.currentFrame = 0
        GLOBAL.loopCount = 1
        GLOBAL.animationIdx = 1
    end

    if GLOBAL.currentFrame <= GLOBAL.frames then
        OUT['0currentFrame'] = GLOBAL.currentFrame
    else
        GLOBAL.currentFrame = GLOBAL.frames
        local animationConfig = DataModule.getActiveAnimationConfig()
        if GLOBAL.loopCount < animationConfig.loopCount then
            GLOBAL.loopCount = GLOBAL.loopCount + 1
            GLOBAL.currentFrame = 0
            GLOBAL.startTick = 0
        else
            -- End of an animation
            if IN.playAlone == false and GLOBAL.animationIdx < #animationList then
                GLOBAL.startTick = 0
                GLOBAL.currentFrame = 0
                GLOBAL.loopCount = 1
                GLOBAL.animationIdx = GLOBAL.animationIdx + 1
            end
        end
    end

    if IN.playAlone == true then
        if IN.animationName ~= '' then
            GLOBAL.animationName = IN.animationName
            DataModule.setActiveAnimationName(GLOBAL.animationName)
            if GLOBAL.lastAnimationName ~= IN.animationName then
                local animationConfig = DataModule.getActiveAnimationConfig()
                GLOBAL.frames = animationConfig['end'] or 100
                GLOBAL.frame = 1000 / (animationConfig.updateInterval or 24)
            end
            GLOBAL.lastAnimationName = IN.animationName
        end
    else
        GLOBAL.animationName = animationList[GLOBAL.animationIdx]
        DataModule.setActiveAnimationName(GLOBAL.animationName)
        if GLOBAL.lastAnimationName ~= animationList[GLOBAL.animationIdx] then
            -- Switch Animation
            local animationConfig = DataModule.getActiveAnimationConfig()

            GLOBAL.frames = animationConfig['end'] or 100

            GLOBAL.frame = 1000 / (animationConfig.updateInterval or 24)
        end
        GLOBAL.lastAnimationName = animationList[GLOBAL.animationIdx]
    end

    OUT.animationName = GLOBAL.animationName

    OUT.progress = GLOBAL.currentFrame / GLOBAL.frames

    local curve = DataModule.getTransformCurveData(interfaceDefinedMap)
    local res = SharedModule.getCurrentPoints(curve, GLOBAL.currentFrame, GLOBAL.animationName)
    -- error(json.encode(res))
    for on, ci in pairs(res) do
        -- local veclen = #ci
        local o = {
            x = 0,
            y = 0,
            z = 0
        }

        for k, pi in pairs(ci.value) do
            if pi.next ~= nil and pi.prev ~= nil then
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
                o[k] = pi.default
            end
        end

        local def = interfaceDefinedMap[on]
        local outputName = nil
        if DEBUG then
            outputName = on
        else
            outputName = def.objectId .. '.' .. def.property
        end

        if OUT[outputName] == nil then
            error('undefined struct property: ' .. outputName)
        end

        if def.type == 'double' then
            OUT[outputName] = o.x
        elseif def.type == 'vec3f' then
            OUT[outputName] = { o.x, o.y, o.z }
        elseif def.type == 'vec2f' then
            OUT[outputName] = { o.x, o.y }
        elseif def.type == 'vec4f' then
            OUT[outputName] = { o.x, o.y, o.z, o.w }
        end
    end

    -- -----------------External interface
    -- Scale in pixel size, the value range is 0-1000, and the default value is 300.
    -- Right: Orientation of Ellie, on the right when the value is 1, on the left when the value is 0, with a default value of 0.
    local right = 0
    if IN.Right == 0 or IN.Right == 1 then
        right = IN.Right
    end

    if IN.Scale >= 0 and IN.Scale <= 1000 then
        local scale = IN.Scale / 300
        local scaleX = scale
        local scaleZ = scale
        if right == 1 then
            scaleX = -scaleX
        end
        OUT.Scale = { scaleX, 1, scaleZ }
    end
    -- For visibility, Hidden when the value is 0, visible when the value is 1, with a default value of 1.
    if IN.Opacity < 0 then
        OUT.Opacity = 0
    elseif OUT.Opacity > 1 then
        OUT.Opacity = 1
    else
        OUT.Opacity = IN.Opacity
    end
    -- HUD Color1
    -- OUT.colorMode_Color1 = UtilModule.blendColors2(IN.Content_IPAIconTransformation_C1, IN.HUD_Content_IPAIconTransformation_C1, IN.HUD)
    -- OUT.colorMode_Color2 = UtilModule.blendColors2(IN.Content_IPAIconTransformation_C2, IN.HUD_Content_IPAIconTransformation_C2, IN.HUD)
    -- OUT.colorMode_Color3 = UtilModule.blendColors2(IN.Content_IPAIconTransformation_C3, IN.HUD_Content_IPAIconTransformation_C3, IN.HUD)
    -- debug
    OUT.colorMode_Color1 = IN.Content_IPAIconTransformation_C1
    OUT.colorMode_Color2 = IN.Content_IPAIconTransformation_C2
    OUT.colorMode_Color3 = IN.Content_IPAIconTransformation_C3
    -- opacity for bg, value range 0-1, default value 0.75.
    if IN.DotOpacity >= 0 and IN.DotOpacity <= 2 then
        OUT.dot_opacity = IN.DotOpacity * OUT.Opacity
    end
    -- contros the size of the bg, value range 0-1, default value 1.
    if IN.DotSize >= 0 and IN.DotSize <= 1 then
        OUT.dot_size = IN.DotSize
    end
    -- Slide: Switch from StartOffset to TargetOffsest, value range 0-1, default value 0.
    local slide = IN.Slide
    -- local slide = 1 - (1 - IN.Slide) * (1 - IN.Slide)
    if slide < 0 then
        slide = 0
    elseif slide > 1 then
        slide = 1
    end

    local _startOffset = IN.StartOffset
    local _targetOffset = {IN.TargetOffset[1], IN.TargetOffset[2]}

    if IN.Right == 1 then
        _startOffset = {IN.StartOffset[1] * -1, IN.StartOffset[2]}
        _targetOffset = {IN.TargetOffset[1] * -1, IN.TargetOffset[2]}
    end

    local x1, y1 = _startOffset[1], _startOffset[2]
    local x2, y2 = _targetOffset[1], _targetOffset[2]

    local dirX = x2 - x1
    local dirY = y2 - y1

    OUT.slide_translation = { x1 + dirX * slide, 0, y1 + dirY * slide }
    OUT.slide = slide
    OUT.start_offset = _startOffset
    OUT.target_offset = _targetOffset
end
