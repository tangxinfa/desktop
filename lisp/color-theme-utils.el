;;; color-theme-utils.el --- Color theme utils  -*- lexical-binding: t; -*-

;; Copyright (C) 2018  tangxinfa

;; Author: tangxinfa <tangxinfa@gmail.com>
;; Keywords: tools

;; This program is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation, either version 3 of the License, or
;; (at your option) any later version.

;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with this program.  If not, see <https://www.gnu.org/licenses/>.

;;; Commentary:

;; Export color theme definitions for other applications to use.

;;; Code:
(require 'color)
(require 'color-theme-approximate)

;; Support hook before/after theme enable and disable.
(defvar before-enable-theme-hook nil
  "Hook run before a color theme is enabled using `enable-theme'.")
(defvar after-enable-theme-hook nil
  "Hook run after a color theme is enabled using `enable-theme'.")
(defun ad-enable-theme-run-hooks (orig-fun theme)
  "Run `before-enable-theme-hook' and `after-enable-theme-hook' before/after enable THEME."
  (let ((result nil))
    (unless (eq theme 'user)
      (run-hooks 'before-enable-theme-hook))
    (setq result (funcall orig-fun theme))
    (unless (eq theme 'user)
      (run-hooks 'after-enable-theme-hook))
    result))
(advice-add 'enable-theme :around #'ad-enable-theme-run-hooks)

(defvar before-disable-theme-hook nil
  "Hook run before a color theme is disabled using `disable-theme'.")
(defvar after-disable-theme-hook nil
  "Hook run after a color theme is disabled using `disable-theme'.")
(defun ad-disable-theme-run-hooks (orig-fun theme)
  "Run `before-disable-theme-hook' `after-disable-theme-hook' before/after disable THEME."
  (let ((result nil))
    (unless (eq theme 'user)
      (run-hooks 'before-disable-theme-hook))
    (setq result (funcall orig-fun theme))
    (unless (eq theme 'user)
      (run-hooks 'after-disable-theme-hook))
    result))
(advice-add 'disable-theme :around #'ad-disable-theme-run-hooks)

;; Disable any enabled themes before enabling a new theme.
(defun ad-enable-theme-disable-all-enabled-themes (theme)
  "Disable any enabled themes before enabling a new THEME.
This prevents overlapping themes; something I would rarely want."
  (unless (eq theme 'user)
    (mapcar #'disable-theme custom-enabled-themes)))
(advice-add 'enable-theme :before #'ad-enable-theme-disable-all-enabled-themes)

;; Export color theme definitions into files.
(defvar color-theme-utils-xresources-file "~/.Xresources"
  "Xresources file to save Emacs color theme definitions.")
(defvar color-theme-utils-rasi-file "~/.emacs.d/.rasi"
  "Rasi file to save Emacs color theme definitions.")
(defvar color-theme-utils-dialog-file "~/.dialogrc"
  "Dialog file to save Emacs color theme definitions.")
(defvar color-theme-utils-dialog-file-template "~/.dialogrc.template"
  "Dialog file template for generate dialog file.")

(defun color-theme-utils--underline-face-color (face)
  "Get underline color of FACE."
  (let ((underline (face-attribute face :underline nil t)))
    (when (listp underline)
      (alist-get :color underline nil))))

(defun color-theme-utils-xresources-save ()
  "Export Emacs color theme to Xresources file."
  (let ((comments "! Generate by color-theme-utils.el")
        (old-contents ""))
    (with-temp-buffer
      (ignore-errors
        (insert-file-contents color-theme-utils-xresources-file)
        (setq old-contents (buffer-string)))
      (flush-lines "^ *Emacs\\.")
      (flush-lines comments)
      (goto-char (point-max))
      (insert (format "\
%s
Emacs.Default.background:              %s\n\
Emacs.Default.foreground:              %s\n\
Emacs.ModeLine.background:             %s\n\
Emacs.ModeLine.foreground:             %s\n\
Emacs.ModeLine.underline:              %s\n\
Emacs.ModeLineHighlight.foreground:    %s\n\
Emacs.ModeLineInactive.background:     %s\n\
Emacs.ModeLineInactive.foreground:     %s\n\
Emacs.ModeLineInactive.underline:      %s\n\
Emacs.Region.background:               %s\n\
Emacs.Region.foreground:               %s\n\
Emacs.Fringe.background:               %s\n\
Emacs.Fringe.foreground:               %s\n\
Emacs.VerticalBorder.background:       %s\n\
Emacs.VerticalBorder.foreground:       %s\n\
Emacs.Menu.background:                 %s\n\
Emacs.Menu.foreground:                 %s\n\
Emacs.Tooltip.background:              %s\n\
Emacs.Tooltip.foreground:              %s\n\
Emacs.Isearch.background:              %s\n\
Emacs.Isearch.foreground:              %s\n\
Emacs.Highlight.background:            %s\n\
Emacs.Shadow.foreground:               %s\n\
Emacs.Cursor.background:               %s\n\
Emacs.Cursor.foreground:               %s\n\
Emacs.Keyword.foreground:              %s\n\
Emacs.Warning.foreground:              %s\n\
Emacs.Error.foreground:                %s\n"
                      comments
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (face-background 'default nil t))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (face-foreground 'default nil t))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-background 'mode-line nil t) (face-background 'default  nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-foreground 'mode-line nil t) (face-foreground 'default nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (color-theme-utils--underline-face-color 'mode-line) (face-background 'default nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-foreground 'mode-line-highlight nil t) (face-foreground 'default nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-background 'mode-line-inactive nil t) (face-background 'default  nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-foreground 'mode-line-inactive nil t) (face-foreground 'default nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (color-theme-utils--underline-face-color 'mode-line-inactive) (face-background 'default nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-background 'region nil t) (face-background 'default  nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-foreground 'region nil t) (face-foreground 'default  nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-background 'fringe nil t) (face-background 'default  nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-foreground 'fringe nil t) (face-foreground 'default  nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-background 'vertical-border nil t) (face-background 'default  nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-foreground 'vertical-border nil t) (face-foreground 'default  nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-background 'menu nil t) (face-background 'default  nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-foreground 'menu nil t) (face-foreground 'default  nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-background 'tooltip nil t) (face-background 'default  nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-foreground 'tooltip nil t) (face-foreground 'default  nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-background 'isearch nil t) (face-background 'default  nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-foreground 'isearch nil t) (face-foreground 'default  nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-background 'highlight nil t) (face-background 'default nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-foreground 'shadow nil t) (face-foreground 'default nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-background 'cursor nil t) (face-background 'default nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-foreground 'cursor nil t) (face-background 'default nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-foreground 'font-lock-keyword-face nil t) (face-foreground 'default nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-foreground 'warning nil t) (face-foreground 'default nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-foreground 'error nil t) (face-foreground 'default nil t)))
                                                       (list 2)))))
      (unless (string= old-contents (buffer-string))
        (let (message-log-max)
          (write-region (point-min) (point-max) color-theme-utils-xresources-file)
          t)))))

(defun color-theme-utils-basic-color-name (color)
  "Basic color name of COLOR."
  (let* ((basic-color-names '("black" "red" "green" "yellow" "blue" "magenta" "cyan" "white"))
         (ca-defined-rgb-list (mapcar #'ca-color-to-rgb basic-color-names))
         (color-value (ca-approximate (ca-color-to-rgb color)))
         (color-index (cl-position color-value ca-defined-rgb-list)))
    (nth color-index basic-color-names)))

(defun color-theme-utils-dialog-save ()
  "Export Emacs color theme to dialog file."
  (let ((old-contents ""))
    (with-temp-buffer
      (ignore-errors
        (insert-file-contents color-theme-utils-dialog-file)
        (setq old-contents (buffer-string))))
    (with-temp-buffer
      (insert-file-contents color-theme-utils-dialog-file-template)
      (let ((emacs-colors
             `((emacs-default-background . ,(color-theme-utils-basic-color-name
                                             (apply #'color-rgb-to-hex
                                                    (nconc (color-name-to-rgb
                                                            (face-background 'default nil t))
                                                           (list 2)))))
               (emacs-default-foreground . ,(color-theme-utils-basic-color-name
                                             (apply #'color-rgb-to-hex
                                                    (nconc (color-name-to-rgb
                                                            (face-foreground 'default nil t))
                                                           (list 2)))))
               (emacs-modeline-background . ,(color-theme-utils-basic-color-name
                                              (apply #'color-rgb-to-hex
                                                     (nconc (color-name-to-rgb
                                                             (or (face-background 'mode-line nil t) (face-background 'default  nil t)))
                                                            (list 2)))))
               (emacs-modeline-foreground . ,(color-theme-utils-basic-color-name
                                              (apply #'color-rgb-to-hex
                                                     (nconc (color-name-to-rgb
                                                             (or (face-foreground 'mode-line nil t) (face-foreground 'default nil t)))
                                                            (list 2)))))
               (emacs-modeline-underline . ,(color-theme-utils-basic-color-name
                                             (apply #'color-rgb-to-hex
                                                    (nconc (color-name-to-rgb
                                                            (or (color-theme-utils--underline-face-color 'mode-line) (face-background 'default nil t)))
                                                           (list 2)))))
               (emacs-modeline-highlight-foreground . ,(color-theme-utils-basic-color-name
                                                        (apply #'color-rgb-to-hex
                                                               (nconc (color-name-to-rgb
                                                                       (or (face-foreground 'mode-line-highlight nil t) (face-foreground 'default nil t)))
                                                                      (list 2)))))
               (emacs-modeline-inactive-background . ,(color-theme-utils-basic-color-name
                                                       (apply #'color-rgb-to-hex
                                                              (nconc (color-name-to-rgb
                                                                      (or (face-background 'mode-line-inactive nil t) (face-background 'default  nil t)))
                                                                     (list 2)))))
               (emacs-modeline-inactive-foreground . ,(color-theme-utils-basic-color-name
                                                       (apply #'color-rgb-to-hex
                                                              (nconc (color-name-to-rgb
                                                                      (or (face-foreground 'mode-line-inactive nil t) (face-foreground 'default nil t)))
                                                                     (list 2)))))
               (emacs-modeline-inactive-underline . ,(color-theme-utils-basic-color-name
                                                      (apply #'color-rgb-to-hex
                                                             (nconc (color-name-to-rgb
                                                                     (or (color-theme-utils--underline-face-color 'mode-line-inactive) (face-background 'default nil t)))
                                                                    (list 2)))))
               (emacs-region-background . ,(color-theme-utils-basic-color-name
                                            (apply #'color-rgb-to-hex
                                                   (nconc (color-name-to-rgb
                                                           (or (face-background 'region nil t) (face-background 'default  nil t)))
                                                          (list 2)))))
               (emacs-region-foreground . ,(color-theme-utils-basic-color-name
                                            (apply #'color-rgb-to-hex
                                                   (nconc (color-name-to-rgb
                                                           (or (face-foreground 'region nil t) (face-foreground 'default  nil t)))
                                                          (list 2)))))
               (emacs-fringe-background . ,(color-theme-utils-basic-color-name
                                            (apply #'color-rgb-to-hex
                                                   (nconc (color-name-to-rgb
                                                           (or (face-background 'fringe nil t) (face-background 'default  nil t)))
                                                          (list 2)))))
               (emacs-fringe-foreground . ,(color-theme-utils-basic-color-name
                                            (apply #'color-rgb-to-hex
                                                   (nconc (color-name-to-rgb
                                                           (or (face-foreground 'fringe nil t) (face-foreground 'default  nil t)))
                                                          (list 2)))))
               (emacs-vertical-border-background . ,(color-theme-utils-basic-color-name
                                                     (apply #'color-rgb-to-hex
                                                            (nconc (color-name-to-rgb
                                                                    (or (face-background 'vertical-border nil t) (face-background 'default  nil t)))
                                                                   (list 2)))))
               (emacs-vertical-border-foreground . ,(color-theme-utils-basic-color-name
                                                     (apply #'color-rgb-to-hex
                                                            (nconc (color-name-to-rgb
                                                                    (or (face-foreground 'vertical-border nil t) (face-foreground 'default  nil t)))
                                                                   (list 2)))))
               (emacs-menu-background . ,(color-theme-utils-basic-color-name
                                          (apply #'color-rgb-to-hex
                                                 (nconc (color-name-to-rgb
                                                         (or (face-background 'menu nil t) (face-background 'default  nil t)))
                                                        (list 2)))))
               (emacs-menu-foreground . ,(color-theme-utils-basic-color-name
                                          (apply #'color-rgb-to-hex
                                                 (nconc (color-name-to-rgb
                                                         (or (face-foreground 'menu nil t) (face-foreground 'default  nil t)))
                                                        (list 2)))))
               (emacs-tooltip-background . ,(color-theme-utils-basic-color-name
                                             (apply #'color-rgb-to-hex
                                                    (nconc (color-name-to-rgb
                                                            (or (face-background 'tooltip nil t) (face-background 'default  nil t)))
                                                           (list 2)))))
               (emacs-tooltip-foreground . ,(color-theme-utils-basic-color-name
                                             (apply #'color-rgb-to-hex
                                                    (nconc (color-name-to-rgb
                                                            (or (face-foreground 'tooltip nil t) (face-foreground 'default  nil t)))
                                                           (list 2)))))
               (emacs-isearch-background . ,(color-theme-utils-basic-color-name
                                             (apply #'color-rgb-to-hex
                                                    (nconc (color-name-to-rgb
                                                            (or (face-background 'isearch nil t) (face-background 'default  nil t)))
                                                           (list 2)))))
               (emacs-isearch-foreground . ,(color-theme-utils-basic-color-name
                                             (apply #'color-rgb-to-hex
                                                    (nconc (color-name-to-rgb
                                                            (or (face-foreground 'isearch nil t) (face-foreground 'default  nil t)))
                                                           (list 2)))))
               (emacs-highlight-background . ,(color-theme-utils-basic-color-name
                                               (apply #'color-rgb-to-hex
                                                      (nconc (color-name-to-rgb
                                                              (or (face-background 'highlight nil t) (face-background 'default nil t)))
                                                             (list 2)))))
               (emacs-shadow-foreground . ,(color-theme-utils-basic-color-name
                                            (apply #'color-rgb-to-hex
                                                   (nconc (color-name-to-rgb
                                                           (or (face-foreground 'shadow nil t) (face-foreground 'default nil t)))
                                                          (list 2)))))
               (emacs-cursor-background . ,(color-theme-utils-basic-color-name
                                            (apply #'color-rgb-to-hex
                                                   (nconc (color-name-to-rgb
                                                           (or (face-background 'cursor nil t) (face-background 'default nil t)))
                                                          (list 2)))))
               (emacs-cursor-foreground . ,(color-theme-utils-basic-color-name
                                            (apply #'color-rgb-to-hex
                                                   (nconc (color-name-to-rgb
                                                           (or (face-foreground 'cursor nil t) (face-background 'default nil t)))
                                                          (list 2)))))
               (emacs-keyword-foreground . ,(color-theme-utils-basic-color-name
                                             (apply #'color-rgb-to-hex
                                                    (nconc (color-name-to-rgb
                                                            (or (face-foreground 'font-lock-keyword-face nil t) (face-foreground 'default nil t)))
                                                           (list 2)))))
               (emacs-warning-foreground . ,(color-theme-utils-basic-color-name
                                             (apply #'color-rgb-to-hex
                                                    (nconc (color-name-to-rgb
                                                            (or (face-foreground 'warning nil t) (face-foreground 'default nil t)))
                                                           (list 2)))))
               (emacs-error-foreground . ,(color-theme-utils-basic-color-name
                                           (apply #'color-rgb-to-hex
                                                  (nconc (color-name-to-rgb
                                                          (or (face-foreground 'error nil t) (face-foreground 'default nil t)))
                                                         (list 2))))))))
        (mapc (lambda (element)
                (let ((key (car element))
                      (value (cdr element)))
                  (goto-char (point-min))
                  (while (re-search-forward (format "{%s}" key) nil t)
                    (replace-match value))))
              emacs-colors)
        (unless (string= old-contents (buffer-string))
          (let (message-log-max)
            (write-region (point-min) (point-max) color-theme-utils-dialog-file)
            t))))))

(defun color-theme-utils-rasi-save ()
  "Export Emacs color theme to rasi file."
  (let ((old-contents ""))
    (with-temp-buffer
      (ignore-errors
        (insert-file-contents color-theme-utils-rasi-file)
        (setq old-contents (buffer-string))))
    (with-temp-buffer
      (insert (format "* {\n\
    emacs-default-background:              %s;\n\
    emacs-default-foreground:              %s;\n\
    emacs-modeline-background:             %s;\n\
    emacs-modeline-foreground:             %s;\n\
    emacs-modeline-underline:              %s;\n\
    emacs-modeline-highlight-foreground:   %s;\n\
    emacs-modeline-inactive-background:    %s;\n\
    emacs-modeline-inactive-foreground:    %s;\n\
    emacs-modeline-inactive-underline:     %s;\n\
    emacs-region-background:               %s;\n\
    emacs-region-foreground:               %s;\n\
    emacs-fringe-background:               %s;\n\
    emacs-fringe-foreground:               %s;\n\
    emacs-vertical-border-background:      %s;\n\
    emacs-vertical-border-foreground:      %s;\n\
    emacs-menu-background:                 %s;\n\
    emacs-menu-foreground:                 %s;\n\
    emacs-tooltip-background:              %s;\n\
    emacs-tooltip-foreground:              %s;\n\
    emacs-isearch-background:              %s;\n\
    emacs-isearch-foreground:              %s;\n\
    emacs-highlight-background:            %s;\n\
    emacs-shadow-foreground:               %s;\n\
    emacs-cursor-background:               %s;\n\
    emacs-cursor-foreground:               %s;\n\
    emacs-keyword-foreground:              %s;\n\
    emacs-warning-foreground:              %s;\n\
    emacs-error-foreground:                %s;\n\
}"
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (face-background 'default nil t))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (face-foreground 'default nil t))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-background 'mode-line nil t) (face-background 'default  nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-foreground 'mode-line nil t) (face-foreground 'default nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (color-theme-utils--underline-face-color 'mode-line) (face-background 'default nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-foreground 'mode-line-highlight nil t) (face-foreground 'default nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-background 'mode-line-inactive nil t) (face-background 'default  nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-foreground 'mode-line-inactive nil t) (face-foreground 'default nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (color-theme-utils--underline-face-color 'mode-line-inactive) (face-background 'default nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-background 'region nil t) (face-background 'default  nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-foreground 'region nil t) (face-foreground 'default  nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-background 'fringe nil t) (face-background 'default  nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-foreground 'fringe nil t) (face-foreground 'default  nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-background 'vertical-border nil t) (face-background 'default  nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-foreground 'vertical-border nil t) (face-foreground 'default  nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-background 'menu nil t) (face-background 'default  nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-foreground 'menu nil t) (face-foreground 'default  nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-background 'tooltip nil t) (face-background 'default  nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-foreground 'tooltip nil t) (face-foreground 'default  nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-background 'isearch nil t) (face-background 'default  nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-foreground 'isearch nil t) (face-foreground 'default  nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-background 'highlight nil t) (face-background 'default nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-foreground 'shadow nil t) (face-foreground 'default nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-background 'cursor nil t) (face-background 'default nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-foreground 'cursor nil t) (face-background 'default nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-foreground 'font-lock-keyword-face nil t) (face-foreground 'default nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-foreground 'warning nil t) (face-foreground 'default nil t)))
                                                       (list 2)))
                      (apply #'color-rgb-to-hex (nconc (color-name-to-rgb
                                                        (or (face-foreground 'error nil t) (face-foreground 'default nil t)))
                                                       (list 2)))))
      (unless (string= old-contents (buffer-string))
        (let (message-log-max)
          (write-region (point-min) (point-max) color-theme-utils-rasi-file)
          t)))))

(defun color-theme-utils-export ()
  "Export currrent color theme definitions."
  (interactive)
  (message "Emacs color theme %s"
           (propertize (symbol-name (or (car custom-enabled-themes) 'default-theme)) 'face 'font-lock-variable-name-face))
  (let ((changed nil))
    (unless (eq nil color-theme-utils-xresources-file)
      (if (color-theme-utils-xresources-save)
          (setq changed t)))
    (unless (eq nil color-theme-utils-rasi-file)
      (if (color-theme-utils-rasi-save)
          (setq changed t)))
    (unless (eq nil color-theme-utils-dialog-file)
      (if (color-theme-utils-dialog-save)
          (setq changed t)))
    (if changed
        (call-process-shell-command "i3-msg exec ~/bin/desktop-on-change"))))

(add-hook 'after-enable-theme-hook #'color-theme-utils-export)

(provide 'color-theme-utils)

;;; color-theme-utils.el ends here
