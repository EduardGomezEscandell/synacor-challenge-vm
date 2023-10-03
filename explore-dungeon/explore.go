package main

import (
	"bytes"
	"fmt"
	"log"
	"math/rand"
	"os"
	"os/exec"
	"regexp"
	"strconv"
	"strings"
	"time"

	"golang.org/x/exp/slices"
)

const duration = 10 * time.Second

func main() {
	ch := make(chan result)
	const n = 50

	for i := 0; i < n; i++ {
		seed := rand.Int63()
		go func() {
			seed := seed
			lineCount, codes, err := explore(seed)
			ch <- result{seed, lineCount, codes, err}
		}()
	}

	results := make([]result, n)
	for i := 0; i < n; i++ {
		results = append(results, <-ch)
	}
	close(ch)

	slices.SortFunc(results, func(a, b result) int {
		if len(a.codes) > len(b.codes) {
			return -1
		}
		if len(a.codes) < len(b.codes) {
			return 1
		}
		if a.execCount > b.execCount {
			return -1
		}
		if a.execCount < b.execCount {
			return 1
		}
		return 0
	})

	for _, r := range results {
		if r.err != nil {
			log.Printf("Seed %d had an error: %v", r.seed, r.err)
			continue
		}
		log.Printf("Seed %d executed %d bytes of code and found codes %s\n", r.seed, r.execCount, r.codes)
	}
}

type result struct {
	seed      int64
	execCount int
	codes     []string
	err       error
}

func explore(seed int64) (execCount int, codes []string, err error) {
	src := rand.NewSource(seed)

	var outBuffer, errBuffer bytes.Buffer

	cmd := exec.Command("../build/Release/vm/cmd/coverage", "../docs/spec/challenge")
	cmd.Stdout = &outBuffer
	cmd.Stderr = &errBuffer

	w, err := cmd.StdinPipe()
	if err != nil {
		return execCount, codes, err
	}
	defer w.Close()

	fmt.Fprintln(w, "take tablet")
	fmt.Fprintln(w, "use tablet")
	if err := cmd.Start(); err != nil {
		return execCount, codes, fmt.Errorf("Could not start: %v", err)
	}

	end := make(chan struct{})
	time.AfterFunc(duration, func() {
		cmd.Process.Signal(os.Interrupt)
		close(end)
	})

loop:
	for {
		select {
		case <-end:
			break loop
		case <-time.After(time.Millisecond):
		}

		answ, ok := select_answer(src, outBuffer.String())
		if ok {
			// fmt.Fprintln(&outBuffer, answ)
			fmt.Fprintln(w, answ)
		}
	}

	time.Sleep(100 * time.Millisecond)

	if err := os.WriteFile(fmt.Sprintf("results/%d", seed), outBuffer.Bytes(), 0644); err != nil {
		log.Printf("Error: %v\n", err)
	}

	if !strings.Contains(errBuffer.String(), "-------------") {
		return execCount, codes, fmt.Errorf("Output buffer does not contain summary: %v", err)
	}

	pattern := regexp.MustCompile(`Covered ([0-9]+) addresses`)
	r := pattern.FindStringSubmatch(errBuffer.String())
	if len(r) != 2 {
		return execCount, codes, fmt.Errorf("Could not parser stderr: %s", errBuffer.String())
	}
	execCount, err = strconv.Atoi(r[1])

	pattern = regexp.MustCompile(`[^\w]([a-zA-Z]{12})[^\w]`)
	match := pattern.FindAllStringSubmatch(outBuffer.String(), -1)
	if len(match) != 0 {
		d := map[string]int{
			// Known collisions
			"dramatically": 1,
			"instructions": 1,
		}
		// Finding unique entries
		for _, m := range match {
			d[m[1]] += 1
		}
		for k, v := range d {
			if v == 1 {
				continue
			}
			delete(d, k)
		}
		for _, m := range match {
			if _, ok := d[m[1]]; ok {
				codes = append(codes, m[1])
			}
		}
	}

	cmd.Wait()

	return execCount, codes, err
}

var relevantLines = regexp.MustCompile("\n- ([^\n]+)\n")

// Biased towards 'go' on purpose
var verbs = []string{"look", "inv", "take", "drop", "use", "go", "go", "go", "go", "go", "go", "go"}

func select_answer(src rand.Source, s string) (string, bool) {
	matches := relevantLines.FindAllStringSubmatch(s, -1)
	if len(matches) == 0 {
		return "", false
	}

	vid := src.Int63() % int64(len(verbs))
	tgt := src.Int63() % int64(len(matches))

	return fmt.Sprintf("%s %s", verbs[vid], matches[tgt][1]), true
}
