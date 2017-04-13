local shader = require "ejoy2d.shader"
local fw = require "ejoy2d.framework"

function fw.EJOY2D_INIT()
--	shader.init()
end

shader.init()

local ejoy2d = {}

local touch = {
	"BEGIN",
	"END",
	"MOVE",
	"CANCEL"
}

local gesture = {
	"PAN",
	"TAP",
	"PINCH",
	"PRESS",
	"DOUBLE_TAP",
}

function ejoy2d.random_generator(seed)
	local generator = {__seed=seed,__seq=0, random=function(self, ...)
		local r, s=fw.random(self.__seed, ...)
		self.__seed = s
		self.__seq = self.__seq + 1
		return r
	end}
	generator:random()
	return generator
end

function ejoy2d.start(callback)
	fw.EJOY2D_UPDATE = assert(callback.update)
	fw.EJOY2D_DRAWFRAME = assert(callback.drawframe)

	fw.EJOY2D_TOUCH = function(x,y,what,id)
		return callback.touch(touch[what],x,y,id)
	end
    fw.EJOY2D_GESTURE = function(what, x1, y1, x2, y2, state)
		return callback.gesture(gesture[what], x1, y1, x2, y2, state)
	end
	fw.EJOY2D_MESSAGE = assert(callback.message)
	fw.EJOY2D_HANDLE_ERROR = assert(callback.handle_error)
	fw.EJOY2D_RESUME = assert(callback.on_resume)
	fw.EJOY2D_PAUSE = assert(callback.on_pause)
	fw.EJOY2D_CLOSE = assert(callback.on_close)

	--optional callbacks
	fw.EJOY2D_VIEW_LAYOUT = callback.view_layout
	fw.EJOY2D_RELOAD = callback.on_reload

	fw.inject()
end

function ejoy2d.clear(color)
	return shader.clear(color)
end

function ejoy2d.define_shader(args)
	return shader.define(args)
end

return ejoy2d
