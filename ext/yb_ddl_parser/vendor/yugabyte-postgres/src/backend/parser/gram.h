/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     IDENT = 258,
     UIDENT = 259,
     FCONST = 260,
     SCONST = 261,
     USCONST = 262,
     BCONST = 263,
     XCONST = 264,
     Op = 265,
     ICONST = 266,
     PARAM = 267,
     TYPECAST = 268,
     DOT_DOT = 269,
     COLON_EQUALS = 270,
     EQUALS_GREATER = 271,
     LESS_EQUALS = 272,
     GREATER_EQUALS = 273,
     NOT_EQUALS = 274,
     ABORT_P = 275,
     ABSOLUTE_P = 276,
     ACCESS = 277,
     ACTION = 278,
     ADD_P = 279,
     ADMIN = 280,
     AFTER = 281,
     AGGREGATE = 282,
     ALL = 283,
     ALSO = 284,
     ALTER = 285,
     ALWAYS = 286,
     ANALYSE = 287,
     ANALYZE = 288,
     AND = 289,
     ANY = 290,
     ARRAY = 291,
     AS = 292,
     ASC = 293,
     ASENSITIVE = 294,
     ASSERTION = 295,
     ASSIGNMENT = 296,
     ASYMMETRIC = 297,
     ATOMIC = 298,
     AT = 299,
     ATTACH = 300,
     ATTRIBUTE = 301,
     AUTHORIZATION = 302,
     BACKWARD = 303,
     BEFORE = 304,
     BEGIN_P = 305,
     BETWEEN = 306,
     BIGINT = 307,
     BINARY = 308,
     BIT = 309,
     BOOLEAN_P = 310,
     BOTH = 311,
     BREADTH = 312,
     BY = 313,
     CACHE = 314,
     CALL = 315,
     CALLED = 316,
     CASCADE = 317,
     CASCADED = 318,
     CASE = 319,
     CAST = 320,
     CATALOG_P = 321,
     CHAIN = 322,
     CHAR_P = 323,
     CHARACTER = 324,
     CHARACTERISTICS = 325,
     CHECK = 326,
     CHECKPOINT = 327,
     CLASS = 328,
     CLOSE = 329,
     CLUSTER = 330,
     COALESCE = 331,
     COLLATE = 332,
     COLLATION = 333,
     COLUMN = 334,
     COLUMNS = 335,
     COMMENT = 336,
     COMMENTS = 337,
     COMMIT = 338,
     COMMITTED = 339,
     COMPRESSION = 340,
     CONCURRENTLY = 341,
     CONFIGURATION = 342,
     CONFLICT = 343,
     CONNECTION = 344,
     CONSTRAINT = 345,
     CONSTRAINTS = 346,
     CONTENT_P = 347,
     CONTINUE_P = 348,
     CONVERSION_P = 349,
     COPY = 350,
     COST = 351,
     CREATE = 352,
     CROSS = 353,
     CSV = 354,
     CUBE = 355,
     CURRENT_P = 356,
     CURRENT_CATALOG = 357,
     CURRENT_DATE = 358,
     CURRENT_ROLE = 359,
     CURRENT_SCHEMA = 360,
     CURRENT_TIME = 361,
     CURRENT_TIMESTAMP = 362,
     CURRENT_USER = 363,
     CURSOR = 364,
     CYCLE = 365,
     DATA_P = 366,
     DATABASE = 367,
     DAY_P = 368,
     DEALLOCATE = 369,
     DEC = 370,
     DECIMAL_P = 371,
     DECLARE = 372,
     DEFAULT = 373,
     DEFAULTS = 374,
     DEFERRABLE = 375,
     DEFERRED = 376,
     DEFINER = 377,
     DELETE_P = 378,
     DELIMITER = 379,
     DELIMITERS = 380,
     DEPENDS = 381,
     DEPTH = 382,
     DESC = 383,
     DETACH = 384,
     DICTIONARY = 385,
     DISABLE_P = 386,
     DISCARD = 387,
     DISTINCT = 388,
     DO = 389,
     DOCUMENT_P = 390,
     DOMAIN_P = 391,
     DOUBLE_P = 392,
     DROP = 393,
     EACH = 394,
     ELSE = 395,
     ENABLE_P = 396,
     ENCODING = 397,
     ENCRYPTED = 398,
     END_P = 399,
     ENUM_P = 400,
     ESCAPE = 401,
     EVENT = 402,
     EXCEPT = 403,
     EXCLUDE = 404,
     EXCLUDING = 405,
     EXCLUSIVE = 406,
     EXECUTE = 407,
     EXISTS = 408,
     EXPLAIN = 409,
     EXPRESSION = 410,
     EXTENSION = 411,
     EXTERNAL = 412,
     EXTRACT = 413,
     FALSE_P = 414,
     FAMILY = 415,
     FETCH = 416,
     FILTER = 417,
     FINALIZE = 418,
     FIRST_P = 419,
     FLOAT_P = 420,
     FOLLOWING = 421,
     FOR = 422,
     FORCE = 423,
     FOREIGN = 424,
     FORWARD = 425,
     FREEZE = 426,
     FROM = 427,
     FULL = 428,
     FUNCTION = 429,
     FUNCTIONS = 430,
     GENERATED = 431,
     GLOBAL = 432,
     GRANT = 433,
     GRANTED = 434,
     GREATEST = 435,
     GROUP_P = 436,
     GROUPING = 437,
     GROUPS = 438,
     HANDLER = 439,
     HAVING = 440,
     HEADER_P = 441,
     HOLD = 442,
     HOUR_P = 443,
     IDENTITY_P = 444,
     IF_P = 445,
     ILIKE = 446,
     IMMEDIATE = 447,
     IMMUTABLE = 448,
     IMPLICIT_P = 449,
     IMPORT_P = 450,
     IN_P = 451,
     INCLUDE = 452,
     INCLUDING = 453,
     INCREMENT = 454,
     INDEX = 455,
     INDEXES = 456,
     INHERIT = 457,
     INHERITS = 458,
     INITIALLY = 459,
     INLINE_P = 460,
     INNER_P = 461,
     INOUT = 462,
     INPUT_P = 463,
     INSENSITIVE = 464,
     INSERT = 465,
     INSTEAD = 466,
     INT_P = 467,
     INTEGER = 468,
     INTERSECT = 469,
     INTERVAL = 470,
     INTO = 471,
     INVOKER = 472,
     IS = 473,
     ISNULL = 474,
     ISOLATION = 475,
     JOIN = 476,
     KEY = 477,
     LABEL = 478,
     LANGUAGE = 479,
     LARGE_P = 480,
     LAST_P = 481,
     LATERAL_P = 482,
     LEADING = 483,
     LEAKPROOF = 484,
     LEAST = 485,
     LEFT = 486,
     LEVEL = 487,
     LIKE = 488,
     LIMIT = 489,
     LISTEN = 490,
     LOAD = 491,
     LOCAL = 492,
     LOCALTIME = 493,
     LOCALTIMESTAMP = 494,
     LOCATION = 495,
     LOCK_P = 496,
     LOCKED = 497,
     LOGGED = 498,
     MAPPING = 499,
     MATCH = 500,
     MATCHED = 501,
     MATERIALIZED = 502,
     MAXVALUE = 503,
     MERGE = 504,
     METHOD = 505,
     MINUTE_P = 506,
     MINVALUE = 507,
     MODE = 508,
     MONTH_P = 509,
     MOVE = 510,
     NAME_P = 511,
     NAMES = 512,
     NATIONAL = 513,
     NATURAL = 514,
     NCHAR = 515,
     NEW = 516,
     NEXT = 517,
     NFC = 518,
     NFD = 519,
     NFKC = 520,
     NFKD = 521,
     NO = 522,
     NONE = 523,
     NORMALIZE = 524,
     NORMALIZED = 525,
     NOT = 526,
     NOTHING = 527,
     NOTIFY = 528,
     NOTNULL = 529,
     NOWAIT = 530,
     NULL_P = 531,
     NULLIF = 532,
     NULLS_P = 533,
     NUMERIC = 534,
     OBJECT_P = 535,
     OF = 536,
     OFF = 537,
     OFFSET = 538,
     OIDS = 539,
     OLD = 540,
     ON = 541,
     ONLY = 542,
     OPERATOR = 543,
     OPTION = 544,
     OPTIONS = 545,
     OR = 546,
     ORDER = 547,
     ORDINALITY = 548,
     OTHERS = 549,
     OUT_P = 550,
     OUTER_P = 551,
     OVER = 552,
     OVERLAPS = 553,
     OVERLAY = 554,
     OVERRIDING = 555,
     OWNED = 556,
     OWNER = 557,
     PARALLEL = 558,
     PARAMETER = 559,
     PARSER = 560,
     PARTIAL = 561,
     PARTITION = 562,
     PASSING = 563,
     PASSWORD = 564,
     PLACING = 565,
     PLANS = 566,
     POLICY = 567,
     POSITION = 568,
     PRECEDING = 569,
     PRECISION = 570,
     PRESERVE = 571,
     PREPARE = 572,
     PREPARED = 573,
     PRIMARY = 574,
     PRIOR = 575,
     PRIVILEGES = 576,
     PROCEDURAL = 577,
     PROCEDURE = 578,
     PROCEDURES = 579,
     PROGRAM = 580,
     PUBLICATION = 581,
     QUOTE = 582,
     RANGE = 583,
     READ = 584,
     REAL = 585,
     REASSIGN = 586,
     RECHECK = 587,
     RECURSIVE = 588,
     REF_P = 589,
     REFERENCES = 590,
     REFERENCING = 591,
     REFRESH = 592,
     REINDEX = 593,
     RELATIVE_P = 594,
     RELEASE = 595,
     RENAME = 596,
     REPEATABLE = 597,
     REPLACE = 598,
     REPLICA = 599,
     RESET = 600,
     RESTART = 601,
     RESTRICT = 602,
     RETURN = 603,
     RETURNING = 604,
     RETURNS = 605,
     REVOKE = 606,
     RIGHT = 607,
     ROLE = 608,
     ROLLBACK = 609,
     ROLLUP = 610,
     ROUTINE = 611,
     ROUTINES = 612,
     ROW = 613,
     ROWS = 614,
     RULE = 615,
     SAVEPOINT = 616,
     SCHEMA = 617,
     SCHEMAS = 618,
     SCROLL = 619,
     SEARCH = 620,
     SECOND_P = 621,
     SECURITY = 622,
     SELECT = 623,
     SEQUENCE = 624,
     SEQUENCES = 625,
     SERIALIZABLE = 626,
     SERVER = 627,
     SESSION = 628,
     SESSION_USER = 629,
     SET = 630,
     SETS = 631,
     SETOF = 632,
     SHARE = 633,
     SHOW = 634,
     SIMILAR = 635,
     SIMPLE = 636,
     SKIP = 637,
     SMALLINT = 638,
     SNAPSHOT = 639,
     SOME = 640,
     SQL_P = 641,
     STABLE = 642,
     STANDALONE_P = 643,
     START = 644,
     STATEMENT = 645,
     STATISTICS = 646,
     STDIN = 647,
     STDOUT = 648,
     STORAGE = 649,
     STORED = 650,
     STRICT_P = 651,
     STRIP_P = 652,
     SUBSCRIPTION = 653,
     SUBSTRING = 654,
     SUPPORT = 655,
     SYMMETRIC = 656,
     SYSID = 657,
     SYSTEM_P = 658,
     TABLE = 659,
     TABLES = 660,
     TABLESAMPLE = 661,
     TABLESPACE = 662,
     TEMP = 663,
     TEMPLATE = 664,
     TEMPORARY = 665,
     TEXT_P = 666,
     THEN = 667,
     TIES = 668,
     TIME = 669,
     TIMESTAMP = 670,
     TO = 671,
     TRAILING = 672,
     TRANSACTION = 673,
     TRANSFORM = 674,
     TREAT = 675,
     TRIGGER = 676,
     TRIM = 677,
     TRUE_P = 678,
     TRUNCATE = 679,
     TRUSTED = 680,
     TYPE_P = 681,
     TYPES_P = 682,
     UESCAPE = 683,
     UNBOUNDED = 684,
     UNCOMMITTED = 685,
     UNENCRYPTED = 686,
     UNION = 687,
     UNIQUE = 688,
     UNKNOWN = 689,
     UNLISTEN = 690,
     UNLOGGED = 691,
     UNTIL = 692,
     UPDATE = 693,
     USER = 694,
     USING = 695,
     VACUUM = 696,
     VALID = 697,
     VALIDATE = 698,
     VALIDATOR = 699,
     VALUE_P = 700,
     VALUES = 701,
     VARCHAR = 702,
     VARIADIC = 703,
     VARYING = 704,
     VERBOSE = 705,
     VERSION_P = 706,
     VIEW = 707,
     VIEWS = 708,
     VOLATILE = 709,
     WHEN = 710,
     WHERE = 711,
     WHITESPACE_P = 712,
     WINDOW = 713,
     WITH = 714,
     WITHIN = 715,
     WITHOUT = 716,
     WORK = 717,
     WRAPPER = 718,
     WRITE = 719,
     XML_P = 720,
     XMLATTRIBUTES = 721,
     XMLCONCAT = 722,
     XMLELEMENT = 723,
     XMLEXISTS = 724,
     XMLFOREST = 725,
     XMLNAMESPACES = 726,
     XMLPARSE = 727,
     XMLPI = 728,
     XMLROOT = 729,
     XMLSERIALIZE = 730,
     XMLTABLE = 731,
     YEAR_P = 732,
     YES_P = 733,
     ZONE = 734,
     _YB_ACCOUNT_P = 735,
     _YB_BACKFILL_P = 736,
     _YB_CHANGE_P = 737,
     _YB_COLOCATED_P = 738,
     _YB_COLOCATION_P = 739,
     _YB_FAILED_LOGIN_ATTEMPTS_P = 740,
     _YB_HASH_P = 741,
     _YB_NONCONCURRENTLY_P = 742,
     _YB_NOPROFILE_P = 743,
     _YB_PROFILE_P = 744,
     _YB_SPLIT_P = 745,
     _YB_TABLEGROUP_P = 746,
     _YB_TABLEGROUPS_P = 747,
     _YB_TABLETS_P = 748,
     _YB_UNLOCK_P = 749,
     NOT_LA = 750,
     NULLS_LA = 751,
     WITH_LA = 752,
     MODE_TYPE_NAME = 753,
     MODE_PLPGSQL_EXPR = 754,
     MODE_PLPGSQL_ASSIGN1 = 755,
     MODE_PLPGSQL_ASSIGN2 = 756,
     MODE_PLPGSQL_ASSIGN3 = 757,
     NO_OPCLASS = 758,
     EXPR_LIST = 759,
     UMINUS = 760
   };
#endif
/* Tokens.  */
#define IDENT 258
#define UIDENT 259
#define FCONST 260
#define SCONST 261
#define USCONST 262
#define BCONST 263
#define XCONST 264
#define Op 265
#define ICONST 266
#define PARAM 267
#define TYPECAST 268
#define DOT_DOT 269
#define COLON_EQUALS 270
#define EQUALS_GREATER 271
#define LESS_EQUALS 272
#define GREATER_EQUALS 273
#define NOT_EQUALS 274
#define ABORT_P 275
#define ABSOLUTE_P 276
#define ACCESS 277
#define ACTION 278
#define ADD_P 279
#define ADMIN 280
#define AFTER 281
#define AGGREGATE 282
#define ALL 283
#define ALSO 284
#define ALTER 285
#define ALWAYS 286
#define ANALYSE 287
#define ANALYZE 288
#define AND 289
#define ANY 290
#define ARRAY 291
#define AS 292
#define ASC 293
#define ASENSITIVE 294
#define ASSERTION 295
#define ASSIGNMENT 296
#define ASYMMETRIC 297
#define ATOMIC 298
#define AT 299
#define ATTACH 300
#define ATTRIBUTE 301
#define AUTHORIZATION 302
#define BACKWARD 303
#define BEFORE 304
#define BEGIN_P 305
#define BETWEEN 306
#define BIGINT 307
#define BINARY 308
#define BIT 309
#define BOOLEAN_P 310
#define BOTH 311
#define BREADTH 312
#define BY 313
#define CACHE 314
#define CALL 315
#define CALLED 316
#define CASCADE 317
#define CASCADED 318
#define CASE 319
#define CAST 320
#define CATALOG_P 321
#define CHAIN 322
#define CHAR_P 323
#define CHARACTER 324
#define CHARACTERISTICS 325
#define CHECK 326
#define CHECKPOINT 327
#define CLASS 328
#define CLOSE 329
#define CLUSTER 330
#define COALESCE 331
#define COLLATE 332
#define COLLATION 333
#define COLUMN 334
#define COLUMNS 335
#define COMMENT 336
#define COMMENTS 337
#define COMMIT 338
#define COMMITTED 339
#define COMPRESSION 340
#define CONCURRENTLY 341
#define CONFIGURATION 342
#define CONFLICT 343
#define CONNECTION 344
#define CONSTRAINT 345
#define CONSTRAINTS 346
#define CONTENT_P 347
#define CONTINUE_P 348
#define CONVERSION_P 349
#define COPY 350
#define COST 351
#define CREATE 352
#define CROSS 353
#define CSV 354
#define CUBE 355
#define CURRENT_P 356
#define CURRENT_CATALOG 357
#define CURRENT_DATE 358
#define CURRENT_ROLE 359
#define CURRENT_SCHEMA 360
#define CURRENT_TIME 361
#define CURRENT_TIMESTAMP 362
#define CURRENT_USER 363
#define CURSOR 364
#define CYCLE 365
#define DATA_P 366
#define DATABASE 367
#define DAY_P 368
#define DEALLOCATE 369
#define DEC 370
#define DECIMAL_P 371
#define DECLARE 372
#define DEFAULT 373
#define DEFAULTS 374
#define DEFERRABLE 375
#define DEFERRED 376
#define DEFINER 377
#define DELETE_P 378
#define DELIMITER 379
#define DELIMITERS 380
#define DEPENDS 381
#define DEPTH 382
#define DESC 383
#define DETACH 384
#define DICTIONARY 385
#define DISABLE_P 386
#define DISCARD 387
#define DISTINCT 388
#define DO 389
#define DOCUMENT_P 390
#define DOMAIN_P 391
#define DOUBLE_P 392
#define DROP 393
#define EACH 394
#define ELSE 395
#define ENABLE_P 396
#define ENCODING 397
#define ENCRYPTED 398
#define END_P 399
#define ENUM_P 400
#define ESCAPE 401
#define EVENT 402
#define EXCEPT 403
#define EXCLUDE 404
#define EXCLUDING 405
#define EXCLUSIVE 406
#define EXECUTE 407
#define EXISTS 408
#define EXPLAIN 409
#define EXPRESSION 410
#define EXTENSION 411
#define EXTERNAL 412
#define EXTRACT 413
#define FALSE_P 414
#define FAMILY 415
#define FETCH 416
#define FILTER 417
#define FINALIZE 418
#define FIRST_P 419
#define FLOAT_P 420
#define FOLLOWING 421
#define FOR 422
#define FORCE 423
#define FOREIGN 424
#define FORWARD 425
#define FREEZE 426
#define FROM 427
#define FULL 428
#define FUNCTION 429
#define FUNCTIONS 430
#define GENERATED 431
#define GLOBAL 432
#define GRANT 433
#define GRANTED 434
#define GREATEST 435
#define GROUP_P 436
#define GROUPING 437
#define GROUPS 438
#define HANDLER 439
#define HAVING 440
#define HEADER_P 441
#define HOLD 442
#define HOUR_P 443
#define IDENTITY_P 444
#define IF_P 445
#define ILIKE 446
#define IMMEDIATE 447
#define IMMUTABLE 448
#define IMPLICIT_P 449
#define IMPORT_P 450
#define IN_P 451
#define INCLUDE 452
#define INCLUDING 453
#define INCREMENT 454
#define INDEX 455
#define INDEXES 456
#define INHERIT 457
#define INHERITS 458
#define INITIALLY 459
#define INLINE_P 460
#define INNER_P 461
#define INOUT 462
#define INPUT_P 463
#define INSENSITIVE 464
#define INSERT 465
#define INSTEAD 466
#define INT_P 467
#define INTEGER 468
#define INTERSECT 469
#define INTERVAL 470
#define INTO 471
#define INVOKER 472
#define IS 473
#define ISNULL 474
#define ISOLATION 475
#define JOIN 476
#define KEY 477
#define LABEL 478
#define LANGUAGE 479
#define LARGE_P 480
#define LAST_P 481
#define LATERAL_P 482
#define LEADING 483
#define LEAKPROOF 484
#define LEAST 485
#define LEFT 486
#define LEVEL 487
#define LIKE 488
#define LIMIT 489
#define LISTEN 490
#define LOAD 491
#define LOCAL 492
#define LOCALTIME 493
#define LOCALTIMESTAMP 494
#define LOCATION 495
#define LOCK_P 496
#define LOCKED 497
#define LOGGED 498
#define MAPPING 499
#define MATCH 500
#define MATCHED 501
#define MATERIALIZED 502
#define MAXVALUE 503
#define MERGE 504
#define METHOD 505
#define MINUTE_P 506
#define MINVALUE 507
#define MODE 508
#define MONTH_P 509
#define MOVE 510
#define NAME_P 511
#define NAMES 512
#define NATIONAL 513
#define NATURAL 514
#define NCHAR 515
#define NEW 516
#define NEXT 517
#define NFC 518
#define NFD 519
#define NFKC 520
#define NFKD 521
#define NO 522
#define NONE 523
#define NORMALIZE 524
#define NORMALIZED 525
#define NOT 526
#define NOTHING 527
#define NOTIFY 528
#define NOTNULL 529
#define NOWAIT 530
#define NULL_P 531
#define NULLIF 532
#define NULLS_P 533
#define NUMERIC 534
#define OBJECT_P 535
#define OF 536
#define OFF 537
#define OFFSET 538
#define OIDS 539
#define OLD 540
#define ON 541
#define ONLY 542
#define OPERATOR 543
#define OPTION 544
#define OPTIONS 545
#define OR 546
#define ORDER 547
#define ORDINALITY 548
#define OTHERS 549
#define OUT_P 550
#define OUTER_P 551
#define OVER 552
#define OVERLAPS 553
#define OVERLAY 554
#define OVERRIDING 555
#define OWNED 556
#define OWNER 557
#define PARALLEL 558
#define PARAMETER 559
#define PARSER 560
#define PARTIAL 561
#define PARTITION 562
#define PASSING 563
#define PASSWORD 564
#define PLACING 565
#define PLANS 566
#define POLICY 567
#define POSITION 568
#define PRECEDING 569
#define PRECISION 570
#define PRESERVE 571
#define PREPARE 572
#define PREPARED 573
#define PRIMARY 574
#define PRIOR 575
#define PRIVILEGES 576
#define PROCEDURAL 577
#define PROCEDURE 578
#define PROCEDURES 579
#define PROGRAM 580
#define PUBLICATION 581
#define QUOTE 582
#define RANGE 583
#define READ 584
#define REAL 585
#define REASSIGN 586
#define RECHECK 587
#define RECURSIVE 588
#define REF_P 589
#define REFERENCES 590
#define REFERENCING 591
#define REFRESH 592
#define REINDEX 593
#define RELATIVE_P 594
#define RELEASE 595
#define RENAME 596
#define REPEATABLE 597
#define REPLACE 598
#define REPLICA 599
#define RESET 600
#define RESTART 601
#define RESTRICT 602
#define RETURN 603
#define RETURNING 604
#define RETURNS 605
#define REVOKE 606
#define RIGHT 607
#define ROLE 608
#define ROLLBACK 609
#define ROLLUP 610
#define ROUTINE 611
#define ROUTINES 612
#define ROW 613
#define ROWS 614
#define RULE 615
#define SAVEPOINT 616
#define SCHEMA 617
#define SCHEMAS 618
#define SCROLL 619
#define SEARCH 620
#define SECOND_P 621
#define SECURITY 622
#define SELECT 623
#define SEQUENCE 624
#define SEQUENCES 625
#define SERIALIZABLE 626
#define SERVER 627
#define SESSION 628
#define SESSION_USER 629
#define SET 630
#define SETS 631
#define SETOF 632
#define SHARE 633
#define SHOW 634
#define SIMILAR 635
#define SIMPLE 636
#define SKIP 637
#define SMALLINT 638
#define SNAPSHOT 639
#define SOME 640
#define SQL_P 641
#define STABLE 642
#define STANDALONE_P 643
#define START 644
#define STATEMENT 645
#define STATISTICS 646
#define STDIN 647
#define STDOUT 648
#define STORAGE 649
#define STORED 650
#define STRICT_P 651
#define STRIP_P 652
#define SUBSCRIPTION 653
#define SUBSTRING 654
#define SUPPORT 655
#define SYMMETRIC 656
#define SYSID 657
#define SYSTEM_P 658
#define TABLE 659
#define TABLES 660
#define TABLESAMPLE 661
#define TABLESPACE 662
#define TEMP 663
#define TEMPLATE 664
#define TEMPORARY 665
#define TEXT_P 666
#define THEN 667
#define TIES 668
#define TIME 669
#define TIMESTAMP 670
#define TO 671
#define TRAILING 672
#define TRANSACTION 673
#define TRANSFORM 674
#define TREAT 675
#define TRIGGER 676
#define TRIM 677
#define TRUE_P 678
#define TRUNCATE 679
#define TRUSTED 680
#define TYPE_P 681
#define TYPES_P 682
#define UESCAPE 683
#define UNBOUNDED 684
#define UNCOMMITTED 685
#define UNENCRYPTED 686
#define UNION 687
#define UNIQUE 688
#define UNKNOWN 689
#define UNLISTEN 690
#define UNLOGGED 691
#define UNTIL 692
#define UPDATE 693
#define USER 694
#define USING 695
#define VACUUM 696
#define VALID 697
#define VALIDATE 698
#define VALIDATOR 699
#define VALUE_P 700
#define VALUES 701
#define VARCHAR 702
#define VARIADIC 703
#define VARYING 704
#define VERBOSE 705
#define VERSION_P 706
#define VIEW 707
#define VIEWS 708
#define VOLATILE 709
#define WHEN 710
#define WHERE 711
#define WHITESPACE_P 712
#define WINDOW 713
#define WITH 714
#define WITHIN 715
#define WITHOUT 716
#define WORK 717
#define WRAPPER 718
#define WRITE 719
#define XML_P 720
#define XMLATTRIBUTES 721
#define XMLCONCAT 722
#define XMLELEMENT 723
#define XMLEXISTS 724
#define XMLFOREST 725
#define XMLNAMESPACES 726
#define XMLPARSE 727
#define XMLPI 728
#define XMLROOT 729
#define XMLSERIALIZE 730
#define XMLTABLE 731
#define YEAR_P 732
#define YES_P 733
#define ZONE 734
#define _YB_ACCOUNT_P 735
#define _YB_BACKFILL_P 736
#define _YB_CHANGE_P 737
#define _YB_COLOCATED_P 738
#define _YB_COLOCATION_P 739
#define _YB_FAILED_LOGIN_ATTEMPTS_P 740
#define _YB_HASH_P 741
#define _YB_NONCONCURRENTLY_P 742
#define _YB_NOPROFILE_P 743
#define _YB_PROFILE_P 744
#define _YB_SPLIT_P 745
#define _YB_TABLEGROUP_P 746
#define _YB_TABLEGROUPS_P 747
#define _YB_TABLETS_P 748
#define _YB_UNLOCK_P 749
#define NOT_LA 750
#define NULLS_LA 751
#define WITH_LA 752
#define MODE_TYPE_NAME 753
#define MODE_PLPGSQL_EXPR 754
#define MODE_PLPGSQL_ASSIGN1 755
#define MODE_PLPGSQL_ASSIGN2 756
#define MODE_PLPGSQL_ASSIGN3 757
#define NO_OPCLASS 758
#define EXPR_LIST 759
#define UMINUS 760




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 274 "gram.y"
{
	core_YYSTYPE core_yystype;
	/* these fields must match core_YYSTYPE: */
	int			ival;
	char	   *str;
	const char *keyword;

	char		chr;
	bool		boolean;
	JoinType	jtype;
	DropBehavior dbehavior;
	OnCommitAction oncommit;
	List	   *list;
	Node	   *node;
	ObjectType	objtype;
	TypeName   *typnam;
	FunctionParameter *fun_param;
	FunctionParameterMode fun_param_mode;
	ObjectWithArgs *objwithargs;
	DefElem	   *defelt;
	SortBy	   *sortby;
	WindowDef  *windef;
	JoinExpr   *jexpr;
	IndexElem  *ielem;
	StatsElem  *selem;
	Alias	   *alias;
	RangeVar   *range;
	IntoClause *into;
	WithClause *with;
	InferClause	*infer;
	OnConflictClause *onconflict;
	A_Indices  *aind;
	ResTarget  *target;
	struct PrivTarget *privtarget;
	AccessPriv *accesspriv;
	struct ImportQual *importqual;
	InsertStmt *istmt;
	VariableSetStmt *vsetstmt;
	PartitionElem *partelem;
	PartitionSpec *partspec;
	PartitionBoundSpec *partboundspec;
	RoleSpec   *rolespec;
	PublicationObjSpec *publicationobjectspec;
	struct SelectLimit *selectlimit;
	SetQuantifier setquantifier;
	struct GroupClause *groupclause;
	MergeWhenClause *mergewhen;
	struct KeyActions *keyactions;
	struct KeyAction *keyaction;

	YbOptSplit *splitopt;
	char *grpopt;
	YbRowBounds *rowbounds;
}
/* Line 1529 of yacc.c.  */
#line 1114 "gram.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


