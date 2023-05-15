type ClusterOptions = {
  nodes?: string[];
  port?: number | string;
  sslContext?: string;
  username?: string;
  password?: string;
};

class Cluster {
  constructor(options?: ClusterOptions);
  connect(keyspace?: string): Session;
}

class UUIDv4 {
  constructor(uuid: string);
  static random(): UUIDv4;
  static fromString(uuid: string): UUIDv4;
  toString(): string;
}

type ScyllaTypeMap = {
  ASCII: string;
  BIGINT: number | bigint;
  BLOB: Buffer;
  BOOLEAN: boolean;
  COUNTER: number | bigint;
  DATE: Date;
  DECIMAL: number;
  DOUBLE: number;
  FLOAT: number;
  INET: string;
  INT: number;
  LIST: Array<any>;
  MAP: Map<any, any>;
  SET: Set<any>;
  TEXT: string;
  TIMESTAMP: Date;
  TIMEUUID: UUIDv4;
  UUID: UUIDv4;
  VARCHAR: string;
  VARINT: number | bigint;
};

export type ScyllaType<T extends keyof ScyllaTypeMap> = ScyllaTypeMap[T];
type ScyllaTypeAny = ScyllaType<keyof ScyllaTypeMap>;

interface Session {
  executeSync<T = Record<string, ScyllaTypeAny>>(
    query: string,
    params?: Array<ScyllaTypeAny>
  ): Array<T>;
}

declare namespace NodeppScyllaDBModule {
  interface ScyllaDBModule {
    Cluster: typeof Cluster;
    UUIDv4: typeof UUIDv4;
  }
}

declare module "scylladb-driver" {
  const exampleModule: NodeppScyllaDBModule.ScyllaDBModule;
  export = exampleModule;
}
