(program maybe_nil ((T Type) (t T))
    (T T) T
    (
      ((maybe_nil t t)       t)
      ((maybe_nil t alf.nil) t)
    )
)

(declare-const = (-> (! Type :var T :implicit) T T Bool))

(declare-const or (-> Bool Bool Bool) :right-assoc-nil)

(program append 
  ((h Bool) (t Bool :list))
  (Bool Bool) Bool
  (
  ((append h t) (or h t))
  )
)

(program replAll 
  ((x Bool) (y Bool) (z Bool) (tail Bool :list))
  (Bool Bool Bool) Bool
  (
  ((replAll x y (or x tail)) (append y (replAll x y tail)))
  ((replAll x y (or z tail)) (append z (replAll x y tail)))
  ((replAll x y z)           z)
  )
)

(declare-rule or_repl-all 
   ((f Bool) (a Bool) (b Bool))
   :premises (f (= a b))
   :args ()
   :conclusion (replAll a b f)
)

(declare-const a Bool)
(declare-const b Bool)
(assume a1 (or a b b a b))
(assume a2 (= a b))
(step a3 (or b b b b b) :rule or_repl-all  :premises (a1 a2))



