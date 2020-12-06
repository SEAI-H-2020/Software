CREATE TABLE users
(
    username TEXT COLLATE pg_catalog."default" NOT NULL,
    email TEXT COLLATE pg_catalog."default" NOT NULL,
    password TEXT COLLATE pg_catalog."default" NOT NULL,
    CONSTRAINT users_pkey PRIMARY KEY (username)
); 

CREATE TABLE measurements
(
    id serial NOT NULL,
    tstamp timestamp without time zone DEFAULT now(),
    temperature real,
    humidity real,
    wind real,
    noise_level real,
    voltage real,
    CONSTRAINT measurements_pkey PRIMARY KEY (id)
);


CREATE TABLE configurations
(
    box integer NOT NULL,
    sync_period integer NOT NULL,
    sample_time integer NOT NULL,
    shutdown_on_wakeup boolean,
    username character varying(25) COLLATE pg_catalog."default",
    CONSTRAINT configurations_pkey PRIMARY KEY (box),
    CONSTRAINT username FOREIGN KEY (username)
        REFERENCES users (username) MATCH SIMPLE
        ON UPDATE NO ACTION
        ON DELETE NO ACTION
        NOT VALID
);
