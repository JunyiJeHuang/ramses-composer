local CurveModule = {}


function CurveModule.GetBezierPoint(t, p0, p1, p2)
    local p0p1 = p0 * (1 - t) + p1 * t
    local p1p2 = p1 * (1 - t) + p2 * t
    local result = p0p1 * (1 - t) + p1p2 * t
    return result
end

function CurveModule.calculateLiner(curKeyFrame, keyFrame1, keyValue1, keyFrame2, keyValue2)
    local offsetKeyFrame = keyFrame2 - keyFrame1;
    local offsetValue = keyValue2 - keyValue1;
    if offsetKeyFrame == 0 then
        return keyValue1
    end
    -- (flg - k1) / (k2 - k1) = (v - v1) / (v2 - v1)
    -- (flg - k1) * (v2 - v1) / (k2 - k1) + v1 = v
    -- return (curKeyFrame - keyFrame1) * (offsetValue / offsetKeyFrame) + keyValue1
    return ((curKeyFrame - keyFrame1) * (keyValue2 - keyValue1)) / (keyFrame2 - keyFrame1) + keyValue1
end

function CurveModule.calculateHermite(curKeyFrame, keyFrame1, keyValue1, keyRightValue, keyFrame2, keyValue2, keyLeftValue)
    if keyFrame2 - keyFrame1 == 0 then
        return keyValue1
    end
    local samplePos = (curKeyFrame - keyFrame1) / (keyFrame2 - keyFrame1)

    local samplePosPow2 = samplePos * samplePos;

    local samplePosPow3 = samplePos * samplePosPow2

    local h1 = 2 * samplePosPow3 - 3 * samplePosPow2 + 1.0
    local h2 = -2 * samplePosPow3 + 3 * samplePosPow2
    local h3 = samplePosPow3 - 2 * samplePosPow2 + samplePos
    local h4 = samplePosPow3 - samplePosPow2

    local value1 = keyValue1 * h1
    local value2 = keyValue2 * h2
    local value3 = (keyRightValue - keyValue1) * h3
    local value4 = (keyLeftValue - keyValue2) * h4

    return (value1 + value2 + value3 + value4)
end

return CurveModule
