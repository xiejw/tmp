package main

import (
	"fmt"
	"os"
	"strings"

	"golang.org/x/term"
)

const (
	ColorCyan  = "\033[36m"
	ColorReset = "\033[0m"
)

func main() {
	items := []string{"apple", "banana", "cherry", "date", "elderberry", "fig", "grape", "honeydew", "kiwi", "lemon"}
	query := ""
	selectedIndex := 0

	// Set terminal to raw mode
	fd := int(os.Stdin.Fd())
	oldState, err := term.MakeRaw(fd)
	if err != nil {
		panic(err)
	}
	defer term.Restore(fd, oldState)

	for {
		// 1. Get Terminal Size
		_, height, _ := term.GetSize(fd)

		// 2. Filter Logic
		var filtered []string
		for _, item := range items {
			if strings.Contains(strings.ToLower(item), strings.ToLower(query)) {
				filtered = append(filtered, item)
			}
		}

		// Keep selection within bounds
		if len(filtered) == 0 {
			selectedIndex = 0
		} else if selectedIndex >= len(filtered) {
			selectedIndex = len(filtered) - 1
		} else if selectedIndex < 0 {
			selectedIndex = 0
		}

		// 3. Render UI (Bottom-up)
		// Clear screen
		fmt.Print("\033[2J")

		// Calculate vertical start position so the query is at the very bottom
		// We need: len(filtered) lines + 1 separator + 1 query line
		numLines := len(filtered)
		maxVisible := height - 2 // Leave room for separator and query
		if numLines > maxVisible {
			numLines = maxVisible
		}

		// Draw Filtered List (moving up from the bottom)
		for i := 0; i < numLines; i++ {
			// Calculate row: (Total Height - 2) for separator/query, then offset by index
			row := height - 2 - (numLines - 1 - i)
			fmt.Printf("\033[%d;1H", row) // Move to row, col 1

			if i == selectedIndex {
				fmt.Printf("%s> %s%s\r", ColorCyan, filtered[i], ColorReset)
			} else {
				fmt.Printf("  %s\r", filtered[i])
			}
		}

		// Draw Separator
		fmt.Printf("\033[%d;1H------------------\r", height-1)

		// Draw Query Line at the very bottom
		fmt.Printf("\033[%d;1H%s> %s%s\r", height, ColorCyan, query, ColorReset)

		// 4. Ensure Cursor is at the end of the query string
		// Position: height (bottom row), column is 3 (for "> ") + length of query + 1
		fmt.Printf("\033[%d;%dH", height, 3+len(query))

		// 5. Read Input
		buf := make([]byte, 3)
		n, _ := os.Stdin.Read(buf)

		if n == 1 {
			char := buf[0]
			switch char {
			case 3: // Ctrl+C
				return
			case 13: // Enter
				term.Restore(fd, oldState)
				if len(filtered) > 0 {
					fmt.Printf("\nSelected: %s\n", filtered[selectedIndex])
				}
				return
			case 127: // Backspace
				if len(query) > 0 {
					query = query[:len(query)-1]
					selectedIndex = 0
				}
			default:
				if char >= 32 && char <= 126 {
					query += string(char)
					selectedIndex = 0
				}
			}
		} else if n == 3 && buf[0] == 27 && buf[1] == 91 {
			switch buf[2] {
			case 65: // Up Arrow
				if selectedIndex > 0 {
					selectedIndex--
				}
			case 66: // Down Arrow
				if selectedIndex < len(filtered)-1 {
					selectedIndex++
				}
			}
		}
	}
}
