--[[

Taken from OAA because OAA is awesome

]]


--[[
partial (fn, ...)

fn - any function reference
... - parameters for partial application

Returns a version of fn which when executed will have `...` parameters passed in to it first

```
function add (a, b)
  return a + b
end

local addToFive = partial(add, 5)

print(addToFive(3)) -- prints 8
```

]]
function partial(fn, arg1, ...)
  if select("#", ...) == 0 then
    return function (...)
      return fn(arg1, ...)
    end
  else
    return partial(partial(fn, arg1), ...)
  end
end
