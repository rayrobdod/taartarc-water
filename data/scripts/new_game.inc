	.rept 4 * 80 + 1
	.byte 0
	.endr

EventScript_ResetAllMapFlags::
	setflag FLAG_SYS_B_DASH
	end

	.rept 3 * 158 + 5
	.byte 0
	.endr
