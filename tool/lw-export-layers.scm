


(define (lw-export-layers img base)
  (define export-no 0)
  (define (export-layer img layer base)
    (define name (car (gimp-item-get-name layer)))
    (define isgroup (< 0 (car (gimp-item-is-group layer))))
    (define hide (char=? (string-ref name 0) #\!))
    (begin (and (not hide)
                (file-png-save-defaults 1 img layer
                (string-append base (car (gimp-image-get-name img)) "-" (number->string export-no) "-" name ".png")
                (string-append (car (gimp-image-get-name img)) "-" (number->string export-no) "-" name ".png")))
           (and isgroup
                (for-each (lambda (l)
                           (begin (set! export-no (+ export-no 1))
                                  (export-layer img l base)))
                          (vector->list (car (cdr (gimp-item-get-children layer))))))))
  (define layers (gimp-image-get-layers img))
  (for-each (lambda (l)
             (begin (export-layer img l (string-append base "/"))
                    (set! export-no (+ export-no 1))))
            (vector->list (car (cdr layers)))))

(script-fu-register "lw-export-layers"
    "Export layers"
    "Exports layers to <imagename>-<layernumber>-<layername>.png.  Traverses groups but does not preserve tree structure.  Saves combined groups to pngs also.  Skips individual layers with names begining with '!'."
    "Lewis Wall"
    "Lewis Wall"
    "2012"
    "RGB RGBA GRAY GRAYA INDEXED INDEXEDA"
    SF-IMAGE "Image" 0
    SF-DIRNAME "Directory" ""
)

(script-fu-menu-register "lw-export-layers" "<Image>/Layer")

(define (lw-export-layers-cmd filename dir)
  (define img (car (gimp-file-load RUN-NONINTERACTIVE filename filename)))
  (lw-export-layers img dir)
  (gimp-image-delete img))

