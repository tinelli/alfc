(program maybe_nil ((T Type) (t T))
    (T T) T
    (
      ((maybe_nil t t)       t)
      ((maybe_nil t alf.nil) t)
    )
)

(declare-const or (-> Bool Bool Bool) :left-assoc-nil)
(declare-const and (-> Bool Bool Bool) :right-assoc-nil)
(declare-const = (-> (! Type :var T :implicit) T T Bool))


(program substitute
  ((T Type) (U Type) (S Type) (x S) (y S) (f (-> T T U)) (a T) (b T) (z U))
  (S S U) U
  (
  ((substitute x y x)       y)
  ((substitute x y (f a b)) (_ (_ (substitute x y f) (substitute x y a)) (substitute x y b)))
  ((substitute x y z)       z)
  )
)


(declare-rule eq-subs
   ((f Bool) (a Bool) (b Bool))
   :premises (f (= a b))
   :args ()
   :conclusion (substitute a b f)
)

(declare-const a Bool)
(declare-const b Bool)
(assume a1 (or a b b (and a a) b))
(assume a2 (= a b))
(step a3 (or b b b (and b b) b) :rule eq-subs :premises (a1 a2))
