(include "../theories/Core.smt2")
(include "../theories/Arith.smt2")

(declare-sort String 0)
(declare-sort RegLan 0)
(declare-type Seq (Type))

(declare-consts <string> String)

(program is_string_type ((U Type))
    (Type) Bool
    (
      ((is_string_type String) true)
      ((is_string_type (Seq U)) true)
      ((is_string_type U) false)
    )
)

; core
(declare-const str.len (-> (! Type :var T :implicit :requires ((is_string_type T) true))
                           T Int))
(declare-const str.++ (-> (! Type :var T :implicit :requires ((is_string_type T) true))
                          T T T) :right-assoc-nil)

; extended functions
(declare-const str.substr (-> (! Type :var T :implicit :requires ((is_string_type T) true))
                              T Int Int T))
(declare-const str.contains (-> (! Type :var T :implicit :requires ((is_string_type T) true))
                                T T Bool))
(declare-const str.replace (-> (! Type :var T :implicit :requires ((is_string_type T) true))
                               T T T T))
(declare-const str.indexof (-> (! Type :var T :implicit :requires ((is_string_type T) true))
                               T T Int Int))
(declare-const str.at (-> String Int String))
(declare-const str.prefixof (-> String String Bool))
(declare-const str.suffixof (-> String String Bool))
(declare-const str.rev (-> String String))
(declare-const str.unit (-> Int String))
(declare-const str.update (-> String String String String))
(declare-const str.to_lower (-> String String))
(declare-const str.to_upper (-> String String))
(declare-const str.to_code (-> String Int))
(declare-const str.from_code (-> Int String))
(declare-const str.is_digit (-> String Bool))
(declare-const str.to_int (-> String Int))
(declare-const str.from_int (-> Int String))
(declare-const str.< (-> String String Bool))
(declare-const str.<= (-> String String Bool))
(declare-const str.replace_all (-> String String String String))
(declare-const str.replace_re (-> String RegLan String String))
(declare-const str.replace_re_all (-> String RegLan String String))

; regular expressions
(declare-const re.allchar RegLan)
(declare-const re.none RegLan)
(declare-const re.all RegLan)
(declare-const re.empty RegLan)
(declare-const str.to_re (-> String RegLan))
(declare-const re.* (-> RegLan RegLan))
(declare-const re.+ (-> RegLan RegLan))
(declare-const re.opt (-> RegLan RegLan))
(declare-const re.comp (-> RegLan RegLan))
(declare-const re.range (-> String String RegLan))
(declare-const re.++ (-> RegLan RegLan RegLan) :right-assoc-nil)
(declare-const re.loop (-> Int Int RegLan RegLan))
(declare-const str.in_re (-> String RegLan Bool))


; Sequences
(declare-const seq.empty (-> (! Type :var T) T))
;(declare f_seq.unit term)
;(define seq.unit (# x term (apply f_seq.unit x)))
;(declare f_seq.nth term)
;(define seq.nth (# x term (# y term (apply (apply f_seq.nth x) y))))
;(declare f_seq.len term)
;(define seq.len (# x term (apply f_seq.len x)))
;(declare f_seq.++ term)
;(define seq.++ (# x term (# y term (apply (apply f_seq.++ x) y))))
;(declare f_seq.extract term)
;(define seq.extract (# x term (# y term (# z term (apply (apply (apply f_seq.extract x) y) z)))))
;(declare f_seq.contains term)
;(define seq.contains (# x term (# y term (apply (apply f_seq.contains x) y))))
;(declare f_seq.replace term)
;(define seq.replace (# x term (# y term (# z term (apply (apply (apply f_seq.replace x) y) z)))))
;(declare f_seq.indexof term)
;(define seq.indexof (# x term (# y term (# z term (apply (apply (apply f_seq.indexof x) y) z)))))
;(declare f_seq.prefixof term)
;(define seq.prefixof (# x term (# y term (apply (apply f_seq.prefixof x) y))))
;(declare f_seq.suffixof term)
;(define seq.suffixof (# x term (# y term (apply (apply f_seq.suffixof x) y))))
;(declare f_seq.rev term)
;(define seq.rev (# x term (apply f_seq.rev x)))
;(declare f_seq.update term)
;(define seq.update (# x term (# y term (# z term (apply (apply (apply f_seq.update x) y) z)))))
; skolem types
;(declare skolem_re_unfold_pos (! t term (! r term (! z mpz term))))