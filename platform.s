define plat0 $00
define plat1 $01
define plat2 $02
define plat3 $03

define playerY $04

lda #$aa
sta plat0
sta plat1
sta plat2
sta plat3

draw_platform:
  ldx #$20
  
  platLoop:
      lsr plat3    ; outputs into carry
      ror plat2    ; input = carry
      ror plat1    ; input = carry
      ror plat0    ; input = carry
  
      bcc hole
    
  plat:
    lda #$02
    sta $04a0,x
    sta $04c0,x
    jmp nextbit
        
  hole:
    lda #$00
    sta $0200,x
  nextbit:
    dex 
    bne platLoop
  
    
