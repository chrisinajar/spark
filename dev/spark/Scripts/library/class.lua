local BaseClass = {}

function BaseClass:init (options)
   if options then
      self.entityId = options.entityId
   end
end

-- class.lua
-- Compatible with Lua 5.1 (not 5.0).
function class (base, init)
   local c = {}    -- a new class instance
   if not init and type(base) == 'function' then
      init = base
      base = BaseClass
   else
      if type(base) ~= 'table' then
         base = BaseClass
      end
   end
   if not init then
      init = function (obj, ...)
         if base and base.init then
            -- make sure that any stuff from the base class is initialized!
            -- not sure why this is in an else statement, i found it this way
            -- perhaps it should just be if, not elseif, we'll see...
            base.init(obj, ...)
         end
      end
   end

   c._base = base

   -- expose a constructor which can be called by <classname>(<args>)
   local mt = {}
   mt.__call = function(s, ...)
      local obj = {}
      setmetatable(obj,c)

      if c.init then
         c.init(obj, ...)
      end
      obj.OnActivate = function (o, ...)
         c.OnActivate(o, ...)
      end
      if c.OnDeactivate then
         obj.OnDeactivate = function (o, ...)
            c.OnDeactivate(o, ...)
         end
      end
      return obj
   end
   mt.__index = function (t, k)
      return base[k]
   end
   setmetatable(c, mt)
   c.__index = c

   c.init = init
   c.is_a = function(self, klass)
      local m = getmetatable(self)
      while m do 
         if m == klass then return true end
         m = m._base
      end
      return false
   end
   return c
end


function shallowcopy(orig)
    if not orig then return {} end
    local orig_type = type(orig)
    local copy
    if orig_type == 'table' then
        copy = {}
        for orig_key, orig_value in pairs(orig) do
            copy[orig_key] = orig_value
        end
    else -- number, string, boolean, etc
        copy = orig
    end
    return copy
end

function deepcopy(orig)
    if not orig then return {} end
    local orig_type = type(orig)
    local copy
    if orig_type == 'table' then
        copy = {}
        for orig_key, orig_value in next, orig, nil do
            copy[deepcopy(orig_key)] = deepcopy(orig_value)
        end
        setmetatable(copy, deepcopy(getmetatable(orig)))
    else -- number, string, boolean, etc
        copy = orig
    end
    return copy
end



return class
