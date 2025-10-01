package patterns

import (
	"embed"
	"fmt"
	"io/fs"
)

//go:embed *.txt
var embeddedPatterns embed.FS

// Open returns a reader for the specified embedded pattern.
// It tries to open the pattern with .txt extension.
// The caller is responsible for closing the returned closer if it's not nil.
func Open(name string) (fs.File, error) {
	patternPath := name + ".txt"
	if _, err := fs.Stat(embeddedPatterns, patternPath); err == nil {
		f, err := embeddedPatterns.Open(patternPath)
		if err != nil {
			return f, fmt.Errorf("open embedded pattern: %w", err)
		}
		return f, nil
	}

	return nil, fmt.Errorf("embedded pattern not found: %q", name)
}
