// Why is oops needed?
//
// - zerolog can have associated metadata but it is not near the root cause.
//   oops is near the root cause which can record more structure.
//
package main

import (
	"errors"
	"os"

	"github.com/rs/zerolog"
	"github.com/rs/zerolog/log"

	"github.com/samber/oops"
	oopszerolog "github.com/samber/oops/loggers/zerolog"
)

func readConfig() error {
	return errors.New("config file not found")
}

func main() {
	log.Logger = log.Output(zerolog.ConsoleWriter{Out: os.Stderr})
	zerolog.ErrorStackMarshaler = oopszerolog.OopsStackMarshaller
	zerolog.ErrorMarshalFunc = oopszerolog.OopsMarshalFunc

	err := readConfig()
	if err != nil {
		err = oops.Trace("req-123").
			With("product_id", "456").
			Wrapf(err, "read config operation failed")

		log.Error().
			Err(err).
			Msg("startup failed")

		os.Exit(1)
	}
}
