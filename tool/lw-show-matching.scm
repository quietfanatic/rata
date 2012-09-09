

(define (lw-show-matching img s v)
  (define (show-matching img layer s v)
    (define (match-substr a b sa sb)
      (or (= sb (string-length b))
          (and (char=? (string-ref a sa) (string-ref b sb))
               (match-substr a b (+ sa 1) (+ sb 1)))))
    (define (search-substr a b sa)
      (and (not (< (- (string-length a) sa) (string-length b)))
           (or (match-substr a b sa 0)
               (search-substr a b (+ sa 1)))))
    (define name (car (gimp-item-get-name layer)))
    (define isgroup (< 0 (car (gimp-item-is-group layer))))
    (if isgroup
        (for-each (lambda (l) (show-matching img l s v))
                  (vector->list (car (cdr (gimp-item-get-children layer)))))
        (if (search-substr name s 0)
            (gimp-item-set-visible layer v))))
 (define layers (gimp-image-get-layers img))
 (gimp-image-undo-group-start img)
 (for-each (lambda (l) (show-matching img l s v))
           (vector->list (car (cdr layers))))
 (gimp-displays-flush)
 (gimp-image-undo-group-end img))

(script-fu-register "lw-show-matching"
    "Show matching..."
    "Show or hide all layers that contain a given string."
    "Lewis Wall"
    "Lewis Wall"
    "2012"
    "RGB RGBA GRAY GRAYA INDEXED INDEXEDA"
    SF-IMAGE "Image" 0
    SF-STRING "Match" ""
    SF-TOGGLE "Visible" TRUE
)

(script-fu-menu-register "lw-show-matching" "<Image>/Layer")


