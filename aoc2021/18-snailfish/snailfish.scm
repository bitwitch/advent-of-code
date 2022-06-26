(define operand_1 '((((4 3) 4) 4) (7 ((8 4) 9))))

(define operand_2 '(1 1))

(define to_explode
  (lambda (num)

    ))

(define to_split
  (lambda (num)
    #t))

(define explode
  (lambda (num)
    num))

(define split
  (lambda (num)
    num))

(define reduce 
  (lambda (num)
    (cond
      ((to_explode num) (reduce (explode num)))
      ((to_split num) (reduce (split num)))
    (else num))))

(define snailfish_add
  (lambda (a b)
    (reduce (cons a (cons b '())))))

(define result
  (snailfish_add operand_1 operand_2))
