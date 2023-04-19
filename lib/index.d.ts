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

interface Session {
  executeSync<T = Record<string, string>>(query: string): Array<T>; // This string should be a row later
}

declare namespace NodeppScyllaDBModule {
  interface ScyllaDBModule {
    Cluster: typeof Cluster;
  }
}

declare module "scylladb-driver" {
  const exampleModule: NodeppScyllaDBModule.ScyllaDBModule;
  export = exampleModule;
}
