NPAGES	equ		$4300

		code	18 bits
		align	4
;------------------------------------------------------------------------------
;------------------------------------------------------------------------------

MMUInit:
		ldi		$t0,#246				; set number of available pages (10 pages already allocated)
		sw		$t0,NPAGES			
		ldi		$t2,#4096				; number of registers to update
		ldi		$t0,#$00
		ldi		$t1,#$000				; regno
.0001:
		mvmap	$x0,$t0,$t1
		add		$t0,$t0,#$01		; increment page numbers
		add		$t1,$t1,#$01
		sub		$t2,$t2,#1
		bne		$t2,$x0,.0001
		; Now setup segment registers
		ldi		$t0,#$0
		ldi		$t1,#$07				; t1 = value to load RWX=111, base = 0
.0002:
		mvseg	$x0,$t1,$t0			; move to the segment register identifed by t0
		add		$t0,$t0,#1			; pick next segment register
		slt		$t2,$t0,#16			; 16 segment regs
		bne		$t2,$x0,.0002
		ret
				
;------------------------------------------------------------------------------
; Find a run of buckets available for mapping virtual to physical addresses.
; The buckets searched are for the current address space, identified by the
; ASID.
;
; Parameters:
;		a0 = pid
;		a1 = number of pages required.
; Modifies:
;		t1,t2,t3,t5
; Returns:
;		v0 = starting bucket number (includes ASID)
;------------------------------------------------------------------------------

FindRun:
	and			$t3,$a0,#$0F			; t3 = pid
	sll			$t3,$t3,#8				; shift into usable position
	ldi			$t1,#0						; t1 = count of consecutive empty buckets
	mov			$t2,$t3						; t2 = map entry number
	ldi			$t5,#255					; max number of pages - 1
	or			$t5,$t5,$t3				; t5 = max in ASID
.0001:
	mvmap		$v0,$x0,$t2				; get map entry into v0
	beq			$v0,$x0,.empty0		; is it empty?
	add			$t2,$t2,#1
	bltu		$t2,$t5,.0001
	mov			$v0,$x0						; got here so no run was found
	ret
.empty0:
	mov			$t3,$t2						; save first empty bucket
.empty1:
	add			$t1,$t1,#1
	bgeu		$t1,$a1,.foundEnough
	add			$t2,$t2,#1				; next bucket
	mvmap		$v0,$x0,$t2				; get map entry
	beq			$v0,$x0,.empty1
	mov			$t1,$x0						; reset counter
	bra			.0001							; go back and find another run
.foundEnough:
	mov			$v0,$t3						; v0 = start of run
	ret

;------------------------------------------------------------------------------
; Parameters:
;		a0 = pid
;		a1 = amount of memory to allocate
; Modifies:
;		t0
; Returns:
;		v0 = pointer to allocated memory in virtual address space.
;------------------------------------------------------------------------------
;
Alloc:
	sub			$sp,$sp,#16
	sw			$ra,[$sp]
	sw			$s1,4[$sp]				; these regs must be saved
	sw			$s2,8[$sp]
	sw			$s3,12[$sp]
	; First check if there are enough pages available in the system.
	add			$v0,$a1,#2047			; v0 = round memory request
	srl			$v0,$v0,#11				; v0 = convert to pages required
	lw			$t0,NPAGES				; check number of pages available
	bleu		$v0,$t0,.enough
	mov			$v0,$x0						; not enough, return null
	bra			.noRun
.enough:
	; There are enough pages, but is there a run long enough in map space?
	sw			$s2,$v0				; save required # pages
	mov			$a1,$v0
	call		FindRun						; find a run of available slots
	beq			$v0,$x0,.noRun
	; Now there are enough pages, and a run available, so allocate
	mov			$s1,$v0						; s1 = start of run
	lw			$s3,NPAGES				; decrease number of pages available in system
	sub			$s3,$s3,$s2
	sw			$s3,NPAGES
	mov			$s3,$v0						; s3 = start of run
.0001:
	palloc	$v0								; allocate a page (cheat and use hardware)
	beq			$v0,$x0,.noRun
	mvmap		$x0,$v0,$s3				; map the page
	add			$s3,$s3,#1				; next bucket
	sub			$s2,$s2,#1
	bne			$s2,$x0,.0001
	sll			$v0,$s1,#11				; v0 = virtual address of allocated mem.
.noRun:
	lw			$ra,[$sp]					; restore saved regs
	lw			s1,4[$sp]
	lw			s2,8[$sp]
	lw			s3,12[$sp]
	add			$sp,$sp,#16
	ret

; Parameters:
;		a0 = pid to allocate for
;
AllocStack:
	palloc	$v0								; allocate a page
	beq			$v0,$x0,.xit			; success?
	sll			$v1,$a0,#8				; 
	or			$v1,$v1,#255			; last page of memory is for stack
	mvmap		$x0,$v0,$v1
.xit:
	ret
