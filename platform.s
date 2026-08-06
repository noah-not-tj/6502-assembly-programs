define plat0 $00
define plat1 $01
define plat2 $02
define plat3 $03

define temp0 $04
define temp1 $05
define temp2 $06
define temp3 $07

define playerY $04

lda #$aa
sta plat0
lda #$ff
sta plat1
sta plat2
lda #$ff
sta plat3

jsr draw_platform
jsr update_platform
jsr delay
jsr draw_platform
jmp end

load_temp:
  lda plat0 
  sta temp0
  lda plat1 
  sta temp1
  lda plat2 
  sta temp2
  lda plat3 
  sta temp3
  rts

draw_platform:
  jsr load_temp
  ldx #$00
  
  platLoop:
      lsr temp3    ; outputs into carry
      ror temp2    ; input = carry
      ror temp1    ; input = carry
      ror temp0    ; input = carry
  
      bcc hole
    
  plat:
    lda #$05         ;green
    sta $04a0,x
    lda #$02
      sta $04c0,x
      sta $04e0,x
      sta $0500,x
    jmp nextbit

  hole:
    lda #$00
    sta $0200,x
  nextbit:
    inx
    cpx #$20
    bne platLoop
    rts
  
update_platform:
  lsr plat3
  ror plat2
  ror plat1
  ror plat0
  rts  

delay:
ldy $02
ldx $02
outer:	
	prep:
	ldx #$02
	inner:
	dex	
	bne inner
dey
bne outer
rts

end:
