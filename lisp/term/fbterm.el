;; -*- no-byte-compile: t -*-

;; Reference: https://www.baidu.com/from=844b/s?pn=20&usm=1&word=%22fbterm.el%22&sa=np&rsv_pq=9964210908518523172&rsv_t=ba27P8k15YIwdXhiUx7sc3SH8sFY2Ocsu3%252BNv4B1b39hS3XwgT3PFcvovQ&ms=1&rqid=9964210908518523172

(load "term/xterm")

(defun terminal-init-fbterm ()
  "Terminal initialization function for linux fbterm."
  (unless (terminal-coding-system)
    (set-terminal-coding-system 'utf-8-unix))

  ;; It can't really display underlines.
  (tty-no-underline)

  (ignore-errors (when gpm-mouse-mode (require 't-mouse) (gpm-mouse-enable)))

  (xterm-register-default-colors xterm-standard-colors))

;;; fbterm.el ends here
